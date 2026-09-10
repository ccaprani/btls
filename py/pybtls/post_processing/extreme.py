"""
Extreme value fitting for PyBTLS block-maximum (BM) and peak-over-threshold
(POT) outputs.

This module promotes the statistical workflow demonstrated in the BM and POT
tutorial notebooks (``docs/source/notebooks/BM.ipynb`` and ``POT.ipynb``) into
library functions: a GEV fit to block maxima, a GPD fit to threshold
excesses, and the corresponding return-level (characteristic value)
computations.

Shape parameter convention
--------------------------
Both fits report the extreme-value shape parameter ``xi`` in the standard
(Coles) convention:

- ``xi < 0``: Weibull type (bounded upper tail),
- ``xi = 0``: Gumbel / exponential type,
- ``xi > 0``: Frechet / Pareto type (heavy upper tail).

Note that SciPy uses ``c = -xi`` for :class:`scipy.stats.genextreme` but
``c = +xi`` for :class:`scipy.stats.genpareto`; the conversion is handled
internally.

Time convention
---------------
PyBTLS simulations count business days; following the tutorial notebooks,
one year is taken as ``DAYS_PER_YEAR = 250`` simulated days when converting
between blocks and years.
"""

import warnings
from dataclasses import dataclass
from typing import Optional, Union

import numpy as np
import pandas as pd
from scipy.stats import genextreme, genpareto

__all__ = ["DAYS_PER_YEAR", "GEVFit", "GPDFit", "fit_gev", "fit_gpd"]

DAYS_PER_YEAR = 250.0
"""Simulated (business) days per year, as in the BM/POT tutorial notebooks."""


def _as_clean_array(data: Union[pd.Series, np.ndarray], name: str) -> np.ndarray:
    """Convert to a 1-D float array with non-finite entries dropped."""
    arr = np.asarray(data, dtype=float).ravel()
    arr = arr[np.isfinite(arr)]
    if arr.size < 2:
        raise ValueError(f"{name} must contain at least 2 finite values.")
    return arr


def _check_fit(shape: float, scale: float, n: int, name: str) -> None:
    """Reject a degenerate fit and warn about an implausible shape."""
    if not np.isfinite(shape) or not np.isfinite(scale) or scale <= 0.0:
        raise ValueError(
            f"The {name} fit did not converge to a valid distribution "
            f"(xi = {shape}, sigma = {scale}) from {n} values."
        )
    if not -0.5 <= shape <= 1.0:
        warnings.warn(
            f"The {name} fit returned xi = {shape:.3f} from {n} values. The "
            "maximum likelihood estimate is unreliable outside "
            "-0.5 <= xi <= 1, so any return level extrapolated from this fit "
            "should be treated as unverified.",
            RuntimeWarning,
            stacklevel=3,
        )


@dataclass
class GEVFit:
    """
    A generalized extreme value (GEV) distribution fitted to block maxima.

    Attributes
    ----------
    shape : float\n
        Shape parameter ``xi`` (Coles convention; SciPy's genextreme
        ``c = -xi``). ``xi < 0`` indicates a bounded (Weibull) upper tail.\n
    loc : float\n
        Location parameter ``mu``, in the load effect's native unit
        (kN or kN·m).\n
    scale : float\n
        Scale parameter ``sigma``, in the load effect's native unit
        (kN or kN·m).\n
    block_size_days : float\n
        The block size in simulated days used to collect the maxima, i.e.
        ``block_size_days + block_size_secs / 86400`` of
        ``OutputConfig.set_BM_output``.\n
    n_blocks_used : int\n
        The number of blocks with a qualifying event, i.e. the sample size
        of the fit. 0 if not recorded.\n
    n_blocks_total : int\n
        The number of blocks supplied, including those that ran without a
        qualifying event (written as 0.0 by the engine). Entries that are
        NaN are treated as absent data rather than empty blocks and are
        excluded. 0 if not recorded, in which case ``return_level`` assumes
        every block was populated.
    """

    shape: float
    loc: float
    scale: float
    block_size_days: float
    n_blocks_used: int = 0
    n_blocks_total: int = 0

    def return_level(self, return_period_years: float) -> float:
        """
        Compute the return level (characteristic value) for a return period.

        The fit describes the blocks that contained a qualifying event, so
        the return period is counted in populated blocks:
        ``m = return_period_years * DAYS_PER_YEAR / block_size_days *
        (n_blocks_used / n_blocks_total)``. The return level ``z`` is the
        quantile of the fitted block-maximum distribution with exceedance
        probability ``1/m``:

        ``z = mu + (sigma / xi) * (y**(-xi) - 1)`` with
        ``y = -log(1 - 1/m)``, or ``z = mu - sigma * log(y)`` for
        ``xi = 0``.

        Parameters
        ----------
        return_period_years : float\n
            The return period in years (e.g. 100 for the 100-year load
            effect). One year is ``DAYS_PER_YEAR`` (250) simulated days.

        Returns
        -------
        float\n
            The return level, in the load effect's native unit (kN or kN·m).
        """
        m = float(return_period_years) * DAYS_PER_YEAR / self.block_size_days
        if self.n_blocks_total:
            # Blocks without a qualifying event were left out of the fit, so
            # the fitted distribution is conditional on a block being
            # populated and only populated blocks may be counted here.
            m *= self.n_blocks_used / self.n_blocks_total
        if m <= 1.0:
            raise ValueError(
                "Fewer than one block with a qualifying event is expected "
                "within the return period; increase return_period_years."
            )
        y = -np.log(1.0 - 1.0 / m)
        if np.isclose(self.shape, 0.0):
            return float(self.loc - self.scale * np.log(y))
        return float(self.loc + (self.scale / self.shape) * (y ** (-self.shape) - 1.0))


@dataclass
class GPDFit:
    """
    A generalized Pareto distribution (GPD) fitted to threshold excesses.

    Attributes
    ----------
    shape : float\n
        Shape parameter ``xi`` (same as SciPy's genpareto ``c``).
        ``xi < 0`` indicates a bounded upper tail.\n
    scale : float\n
        Scale parameter ``sigma`` of the excess distribution, in the load
        effect's native unit (kN or kN·m).\n
    threshold : float\n
        The threshold ``u`` above which excesses were fitted, in the load
        effect's native unit (kN or kN·m).\n
    n_exceedances : int\n
        The number of peaks above the threshold used in the fit.\n
    n_peaks_per_year : float or None\n
        The average number of threshold exceedances per year, needed for
        return-level computation. None if not provided.
    """

    shape: float
    scale: float
    threshold: float
    n_exceedances: int
    n_peaks_per_year: Optional[float] = None

    def return_level(self, return_period_years: float) -> float:
        """
        Compute the return level (characteristic value) for a return period.

        With ``m = n_peaks_per_year * return_period_years`` threshold
        exceedances expected within the return period, the return level is

        ``z = u + (sigma / xi) * (m**xi - 1)``, or ``z = u + sigma * log(m)``
        for ``xi = 0``.

        Parameters
        ----------
        return_period_years : float\n
            The return period in years (e.g. 100 for the 100-year load
            effect). One year is ``DAYS_PER_YEAR`` (250) simulated days.

        Returns
        -------
        float\n
            The return level, in the load effect's native unit (kN or kN·m).
        """
        if self.n_peaks_per_year is None:
            raise ValueError(
                "n_peaks_per_year is required to compute a return level; "
                "pass it to fit_gpd()."
            )
        m = self.n_peaks_per_year * float(return_period_years)
        if m <= 1.0:
            raise ValueError(
                "Fewer than one threshold exceedance is expected within the "
                "return period; the return level is below the threshold."
            )
        if np.isclose(self.shape, 0.0):
            return float(self.threshold + self.scale * np.log(m))
        return float(self.threshold + (self.scale / self.shape) * (m**self.shape - 1.0))


def fit_gev(
    block_maxima: Union[pd.Series, np.ndarray], block_size_days: float
) -> GEVFit:
    """
    Fit a GEV distribution to block maxima by maximum likelihood.

    This reproduces the BM tutorial notebook workflow: the maxima are fitted
    with :func:`scipy.stats.genextreme.fit` and the SciPy shape ``c`` is
    negated to the standard convention ``xi = -c``.

    Blocks without a qualifying event are dropped before fitting. In a
    ``read_BM_S`` column they take two forms: NaN, where the truck-count
    bucket is missing from that block's row, and a literal 0.0, where the
    engine created the bucket (because the block held a larger event) but
    never filled it. Neither is a block maximum, so both are excluded and
    counted as empty blocks; ``GEVFit.return_level`` then scales the return
    period by the populated fraction, since the fit describes the populated
    blocks only.

    Parameters
    ----------
    block_maxima : pd.Series or np.ndarray\n
        1-D block maxima in the load effect's native unit (kN or kN·m),
        e.g. one truck-count column of a ``read_data("BM_summary")``
        DataFrame. Pass the whole column, empty blocks included: its length
        is taken as the number of blocks simulated.\n
    block_size_days : float\n
        The block size in simulated days, i.e. ``block_size_days +
        block_size_secs / 86400`` of ``OutputConfig.set_BM_output``
        (``1 / 24`` for hourly blocks). One year is ``DAYS_PER_YEAR`` (250)
        simulated days.

    Returns
    -------
    GEVFit\n
        The fitted parameters, with a ``return_level`` method.

    Example
    -------
    >>> bm = output["BM_summary"]["BM_S_20_Eff_1"]["1-Truck Event"]
    >>> fit = fit_gev(bm, block_size_days=250)
    >>> fit.return_level(100)  # 100-year characteristic value
    """
    if block_size_days <= 0.0:
        raise ValueError(
            "block_size_days must be positive; for blocks configured with "
            "block_size_secs, pass block_size_days + block_size_secs / 86400 "
            "(1 / 24 for hourly blocks)."
        )
    blocks = np.asarray(block_maxima, dtype=float).ravel()
    # 0.0 marks a block that ran but held no qualifying event: the engine
    # writes one value per bucket from a default-constructed CEffect
    # (CBlockMaxManager::WriteSummaryFiles). Such a block still counts
    # towards the exceedance rate, so it is excluded from the fit but kept
    # in n_blocks_total. NaN is absent data instead of an observed empty
    # block -- pandas pads it in where a block row carries fewer buckets
    # than a later one -- so it is dropped from both counts.
    finite = blocks[np.isfinite(blocks)]
    data = finite[finite != 0.0]
    if data.size < 4:
        raise ValueError(
            "block_maxima must contain at least 4 blocks with a qualifying "
            f"event to identify the three GEV parameters; got {data.size}."
        )
    # Gumbel moment estimates as starting values; SciPy's default starting
    # point makes the MLE optimization unreliable for GEV data.
    scale0 = np.std(data) * np.sqrt(6.0) / np.pi
    loc0 = np.mean(data) - 0.5772156649 * scale0
    c, loc, scale = genextreme.fit(data, 0.1, loc=loc0, scale=scale0)
    _check_fit(-c, scale, data.size, "GEV")
    return GEVFit(
        shape=-c,
        loc=loc,
        scale=scale,
        block_size_days=float(block_size_days),
        n_blocks_used=int(data.size),
        n_blocks_total=int(finite.size),
    )


def fit_gpd(
    peaks: Union[pd.Series, np.ndarray],
    threshold: float,
    n_peaks_per_year: Optional[float] = None,
) -> GPDFit:
    """
    Fit a GPD to peak-over-threshold excesses by maximum likelihood.

    This reproduces the POT tutorial notebook workflow: peaks above the
    threshold are reduced to excesses ``y = peak - threshold`` and fitted
    with :func:`scipy.stats.genpareto.fit` with the location fixed at zero
    (``floc=0``). The reported ``shape`` equals SciPy's ``c`` (already the
    standard ``xi`` convention). Non-finite values are dropped before
    fitting.

    Parameters
    ----------
    peaks : pd.Series or np.ndarray\n
        1-D peak values in the load effect's native unit (kN or kN·m),
        e.g. the "Peak Value" column of a ``read_data("POT_summary")``
        DataFrame. Values not exceeding the threshold are ignored.\n
    threshold : float\n
        The POT threshold ``u``, in the load effect's native unit
        (kN or kN·m). May be higher than the recording threshold used in
        the simulation.\n
    n_peaks_per_year : float, optional\n
        The average number of threshold exceedances per year (one year is
        ``DAYS_PER_YEAR`` (250) simulated days). Required for
        ``return_level``.

    Returns
    -------
    GPDFit\n
        The fitted parameters, with a ``return_level`` method.

    Example
    -------
    >>> peaks = output["POT_summary"]["PT_S_20_Eff_1"]["Peak Value"]
    >>> no_years = no_day / 250
    >>> n_rate = (peaks > 2400.0).sum() / no_years
    >>> fit = fit_gpd(peaks, threshold=2400.0, n_peaks_per_year=n_rate)
    >>> fit.return_level(100)  # 100-year characteristic value
    """
    data = _as_clean_array(peaks, "peaks")
    excess = data[data > threshold] - threshold
    if excess.size < 3:
        raise ValueError(
            "peaks must contain at least 3 values above the threshold to "
            f"identify the two GPD parameters; got {excess.size}."
        )
    c, _, scale = genpareto.fit(excess, floc=0.0)
    _check_fit(c, scale, excess.size, "GPD")
    return GPDFit(
        shape=c,
        scale=scale,
        threshold=float(threshold),
        n_exceedances=int(excess.size),
        n_peaks_per_year=(
            float(n_peaks_per_year) if n_peaks_per_year is not None else None
        ),
    )
