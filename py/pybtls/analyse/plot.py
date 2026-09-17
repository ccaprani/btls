"""
Diagnostic plots for the extreme value fits of :mod:`pybtls.analyse.extreme`.

A fit on its own yields a number; these show whether it describes the data
(``plot_return_level``, ``plot_qq``) and which threshold a GPD fit should be
given (``plot_mean_residual_life``, ``plot_parameter_stability``).
"""

from pathlib import Path
from typing import Union

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
from scipy.stats import genextreme, genpareto

from .extreme import DAYS_PER_YEAR, GEVFit, GPDFit

__all__ = [
    "plot_return_level",
    "plot_qq",
    "plot_mean_residual_life",
    "plot_parameter_stability",
]


def _start_figure(nrows: int = 1, figsize=(8, 5)):
    plt.rcParams["font.family"] = "Times New Roman"
    plt.rcParams["font.size"] = 16
    plt.rcParams["mathtext.fontset"] = "stix"
    return plt.subplots(nrows, 1, figsize=figsize, sharex=nrows > 1)


def _finish_figure(fig, save_to: Path) -> None:
    fig.tight_layout()
    if save_to is not None:
        fig.savefig(save_to, format="png", dpi=500, pad_inches=0.1, bbox_inches="tight")
    else:
        plt.show()


def _fit_sample(fit, data) -> np.ndarray:
    """The values the fit describes, sorted ascending: the populated block
    maxima of a GEV fit (the 0.0 of an empty block is not a maximum, as in
    ``fit_gev``), the threshold excesses of a GPD fit."""

    values = np.asarray(data, dtype=float).ravel()
    values = values[np.isfinite(values)]
    if isinstance(fit, GEVFit):
        values = values[values != 0.0]
    else:
        values = values[values > fit.threshold] - fit.threshold
    if values.size < 2:
        raise ValueError(
            "data holds fewer than 2 values that the fit describes (populated "
            "blocks for a GEV fit, threshold exceedances for a GPD fit)."
        )
    return np.sort(values)


def _plotting_positions(n: int) -> np.ndarray:
    """Weibull plotting positions i / (n + 1): the non-exceedance probability
    assigned to each order statistic."""

    return np.arange(1, n + 1) / (n + 1.0)


def _empirical_return_years(fit, n: int) -> np.ndarray:
    """The return period of each order statistic, in years, inverting the
    return period -> event count conversion of ``fit.return_level``."""

    count = 1.0 / (1.0 - _plotting_positions(n))
    if isinstance(fit, GEVFit):
        populated = (
            fit.n_blocks_used / fit.n_blocks_total if fit.n_blocks_total else 1.0
        )
        return count * fit.block_size_days / (DAYS_PER_YEAR * populated)
    if fit.n_peaks_per_year is None:
        raise ValueError(
            "n_peaks_per_year is required to place the observations on a "
            "return period axis; pass it to fit_gpd()."
        )
    return count / fit.n_peaks_per_year


def plot_return_level(
    fit: Union[GEVFit, GPDFit],
    data: Union[pd.Series, np.ndarray] = None,
    return_period_years: Union[np.ndarray, list] = None,
    save_to: Path = None,
) -> None:
    """
    Plot the fitted return level against the return period, with the
    observations it was fitted to.

    The return level curve is the characteristic value the fit extrapolates
    for each return period; the observations are placed on the same axis by
    their Weibull plotting positions ``i / (n + 1)``. A fit that describes
    its data follows the observations and extends them smoothly: a curve
    that bends away from the last observations is extrapolating beyond what
    the data support.

    Parameters
    ----------
    fit : GEVFit or GPDFit\n
        The fit, from ``fit_gev`` or ``fit_gpd``. A GPD fit needs its
        ``n_peaks_per_year``.

    data : pd.Series or np.ndarray, optional\n
        The sample the fit was made from, in the same form it was passed to
        the fit function: the block maxima for a GEV fit (empty blocks
        included), the peak values for a GPD fit. Without it only the curve
        is drawn.

    return_period_years : array-like, optional\n
        The return periods to draw the curve at. The default spans the
        return periods of the observations up to 10 times the longest, or
        1 to 1000 years when no data is given. A GPD return period that
        expects fewer than one exceedance is dropped, since the return
        level is then below the threshold.

    save_to : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    values = None if data is None else _fit_sample(fit, data)

    if return_period_years is None:
        if values is None:
            low, high = 1.0, 1000.0
        else:
            years = _empirical_return_years(fit, values.size)
            low, high = years[0], years[-1] * 10.0
        return_period_years = np.logspace(np.log10(low), np.log10(high), 200)
    periods = np.asarray(return_period_years, dtype=float).ravel()
    if isinstance(fit, GPDFit) and fit.n_peaks_per_year is not None:
        periods = periods[fit.n_peaks_per_year * periods > 1.0]
    levels = np.array([fit.return_level(period) for period in periods])

    fig, ax = _start_figure()
    ax.plot(periods, levels, color="black", label="Fitted return level")
    if values is not None:
        observed = values if isinstance(fit, GEVFit) else values + fit.threshold
        ax.scatter(
            _empirical_return_years(fit, values.size),
            observed,
            s=18,
            facecolors="none",
            edgecolors="gray",
            label="Observations",
        )
    ax.set_xscale("log")
    ax.set_xlabel("Return Period (years)")
    ax.set_ylabel("Return Level")
    ax.legend()

    _finish_figure(fig, save_to)

    return None


def plot_qq(
    fit: Union[GEVFit, GPDFit],
    data: Union[pd.Series, np.ndarray],
    save_to: Path = None,
) -> None:
    """
    Plot the observations against the quantiles of the fitted distribution.

    Each order statistic is plotted against the quantile the fit assigns to
    its plotting position, so a fit that describes the data lies on the
    45-degree line. Systematic departure in the upper right is the one that
    matters: it is the tail the return levels come from.

    Parameters
    ----------
    fit : GEVFit or GPDFit\n
        The fit, from ``fit_gev`` or ``fit_gpd``.

    data : pd.Series or np.ndarray\n
        The sample the fit was made from, as passed to the fit function.

    save_to : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    values = _fit_sample(fit, data)
    probabilities = _plotting_positions(values.size)
    if isinstance(fit, GEVFit):
        model = genextreme.ppf(probabilities, -fit.shape, loc=fit.loc, scale=fit.scale)
        observed = values
    else:
        model = (
            genpareto.ppf(probabilities, fit.shape, loc=0.0, scale=fit.scale)
            + fit.threshold
        )
        observed = values + fit.threshold

    fig, ax = _start_figure(figsize=(6, 6))
    ax.scatter(model, observed, s=18, facecolors="none", edgecolors="gray")
    limits = [min(model[0], observed[0]), max(model[-1], observed[-1])]
    ax.plot(limits, limits, color="black", linewidth=1)
    ax.set_xlabel("Fitted Quantile")
    ax.set_ylabel("Observed Value")

    _finish_figure(fig, save_to)

    return None


def _threshold_grid(peaks: np.ndarray, thresholds, min_exceedances: int):
    """The thresholds to evaluate: the given ones, or 50 spanning the peaks,
    each keeping at least ``min_exceedances`` exceedances."""

    if thresholds is None:
        thresholds = np.linspace(peaks.min(), np.quantile(peaks, 0.95), 50)
    thresholds = np.asarray(thresholds, dtype=float).ravel()
    usable = np.array(
        [(peaks > u).sum() >= min_exceedances for u in thresholds], dtype=bool
    )
    if not usable.any():
        raise ValueError(
            f"No threshold leaves {min_exceedances} exceedances; the peaks "
            "cover too small a range or there are too few of them."
        )
    return thresholds[usable]


def plot_mean_residual_life(
    peaks: Union[pd.Series, np.ndarray],
    thresholds: Union[np.ndarray, list] = None,
    save_to: Path = None,
) -> None:
    """
    Plot the mean excess over a threshold against the threshold, to choose
    the threshold of a GPD fit.

    Above a threshold at which the GPD holds, the mean excess is linear in
    the threshold, so the lowest threshold from which the plot stays linear
    (within its confidence interval) is the one to fit from: lower wastes
    the asymptotic argument, higher wastes data. The interval is the 95%
    interval of the mean, ``1.96 * s / sqrt(k)`` over ``k`` exceedances, so
    it widens as the exceedances run out on the right.

    Parameters
    ----------
    peaks : pd.Series or np.ndarray\n
        1-D peak values in the load effect's native unit (kN or kN·m), e.g.
        the "Peak Value" column of a ``read_data("POT_summary")`` frame -
        the same values ``fit_gpd`` takes.

    thresholds : array-like, optional\n
        The thresholds to evaluate. The default is 50 evenly spaced from the
        smallest peak to their 95th percentile. A threshold with fewer than
        10 exceedances is dropped.

    save_to : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    values = np.asarray(peaks, dtype=float).ravel()
    values = values[np.isfinite(values)]
    grid = _threshold_grid(values, thresholds, min_exceedances=10)

    mean_excess = np.empty(grid.size)
    half_width = np.empty(grid.size)
    for i, threshold in enumerate(grid):
        excess = values[values > threshold] - threshold
        mean_excess[i] = excess.mean()
        half_width[i] = 1.96 * excess.std(ddof=1) / np.sqrt(excess.size)

    fig, ax = _start_figure()
    ax.plot(grid, mean_excess, color="black")
    ax.fill_between(
        grid,
        mean_excess - half_width,
        mean_excess + half_width,
        color="gray",
        alpha=0.3,
        label="95% interval",
    )
    ax.set_xlabel("Threshold")
    ax.set_ylabel("Mean Excess")
    ax.legend()

    _finish_figure(fig, save_to)

    return None


def _gpd_covariance(excess: np.ndarray, shape: float, scale: float):
    """Approximate covariance of (xi, sigma), from the observed information
    computed by central differences of the negative log-likelihood. None when
    the information matrix is singular or gives a negative variance, as it
    does where the maximum likelihood estimate is on a boundary."""

    def negative_log_likelihood(parameters) -> float:
        xi, sigma = parameters
        if sigma <= 0.0:
            return np.inf
        return -np.sum(genpareto.logpdf(excess, xi, loc=0.0, scale=sigma))

    centre = np.array([shape, scale])
    step = np.array([max(abs(shape), 1.0) * 1e-3, scale * 1e-3])
    hessian = np.empty((2, 2))
    for i in range(2):
        for j in range(2):
            offset_i = np.zeros(2)
            offset_j = np.zeros(2)
            offset_i[i] = step[i]
            offset_j[j] = step[j]
            hessian[i, j] = (
                negative_log_likelihood(centre + offset_i + offset_j)
                - negative_log_likelihood(centre + offset_i - offset_j)
                - negative_log_likelihood(centre - offset_i + offset_j)
                + negative_log_likelihood(centre - offset_i - offset_j)
            ) / (4.0 * step[i] * step[j])
    if not np.all(np.isfinite(hessian)):
        return None
    try:
        covariance = np.linalg.inv(hessian)
    except np.linalg.LinAlgError:
        return None
    if np.any(np.diag(covariance) <= 0.0):
        return None
    return covariance


def plot_parameter_stability(
    peaks: Union[pd.Series, np.ndarray],
    thresholds: Union[np.ndarray, list] = None,
    save_to: Path = None,
) -> None:
    """
    Plot the GPD parameters against the threshold they were fitted from, to
    choose the threshold of a GPD fit.

    Above a threshold at which the GPD holds, the shape ``xi`` and the
    modified scale ``sigma* = sigma - xi * u`` do not change with the
    threshold ``u``, so the lowest threshold from which both stay constant
    (within their confidence intervals) is the one to fit from. This is the
    same judgement as ``plot_mean_residual_life`` makes, on the fitted
    parameters rather than the sample mean; read them together.

    The intervals are 95% Wald intervals from the observed information, with
    the modified scale's variance by the delta method. They are omitted for
    a threshold whose information matrix is singular.

    Parameters
    ----------
    peaks : pd.Series or np.ndarray\n
        1-D peak values in the load effect's native unit (kN or kN·m), the
        same values ``fit_gpd`` takes.

    thresholds : array-like, optional\n
        The thresholds to evaluate. The default is 50 evenly spaced from the
        smallest peak to their 95th percentile. A threshold with fewer than
        30 exceedances is dropped: the two fitted parameters are too
        uncertain to compare below that.

    save_to : Path, optional\n
        The path to save the plot to. \n
        If not specified, the plot will be displayed on screen.

    Returns
    -------
    None
    """

    values = np.asarray(peaks, dtype=float).ravel()
    values = values[np.isfinite(values)]
    grid = _threshold_grid(values, thresholds, min_exceedances=30)

    shapes = np.empty(grid.size)
    modified_scales = np.empty(grid.size)
    shape_half_widths = np.full(grid.size, np.nan)
    scale_half_widths = np.full(grid.size, np.nan)
    for i, threshold in enumerate(grid):
        excess = values[values > threshold] - threshold
        shape, _, scale = genpareto.fit(excess, floc=0.0)
        shapes[i] = shape
        modified_scales[i] = scale - shape * threshold
        covariance = _gpd_covariance(excess, shape, scale)
        if covariance is not None:
            shape_half_widths[i] = 1.96 * np.sqrt(covariance[0, 0])
            # delta method for sigma* = sigma - xi * u, gradient (-u, 1)
            variance = (
                threshold**2 * covariance[0, 0]
                - 2.0 * threshold * covariance[0, 1]
                + covariance[1, 1]
            )
            scale_half_widths[i] = 1.96 * np.sqrt(variance) if variance > 0 else np.nan

    fig, axes = _start_figure(nrows=2, figsize=(8, 8))
    for ax, value, half_width, label in (
        (axes[0], modified_scales, scale_half_widths, "Modified Scale"),
        (axes[1], shapes, shape_half_widths, "Shape"),
    ):
        ax.plot(grid, value, color="black")
        ax.fill_between(
            grid,
            value - half_width,
            value + half_width,
            color="gray",
            alpha=0.3,
        )
        ax.set_ylabel(label)
    axes[1].set_xlabel("Threshold")

    _finish_figure(fig, save_to)

    return None
