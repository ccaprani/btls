"""
Tests for pybtls.analyse: extreme value fitting (fit_gev / fit_gpd) and its
diagnostic plots.
"""

import matplotlib

matplotlib.use("Agg")

import numpy as np
import pandas as pd
import pytest
from scipy.stats import genextreme, genpareto

import pybtls as pb
from pybtls.analyse import (
    fit_gev,
    fit_gpd,
    GEVFit,
    GPDFit,
    plot_mean_residual_life,
    plot_parameter_stability,
    plot_qq,
    plot_return_level,
)
from pybtls.analyse.plot import _empirical_return_years, _threshold_grid


def test_module_is_exposed_on_package():
    assert pb.analyse.fit_gev is fit_gev
    assert pb.analyse.fit_gpd is fit_gpd


def test_fit_gev_recovers_known_parameters():
    # True GEV: xi = -0.2 (Weibull tail), mu = 1000, sigma = 100.
    # SciPy's genextreme uses c = -xi.
    xi, mu, sigma = -0.2, 1000.0, 100.0
    rng = np.random.default_rng(0)
    data = genextreme.rvs(c=-xi, loc=mu, scale=sigma, size=2000, random_state=rng)

    fit = fit_gev(data, block_size_days=250)

    assert isinstance(fit, GEVFit)
    assert fit.shape == pytest.approx(xi, abs=0.05)
    assert fit.loc == pytest.approx(mu, rel=0.02)
    assert fit.scale == pytest.approx(sigma, rel=0.05)
    assert fit.block_size_days == 250.0


def test_fit_gpd_recovers_known_parameters():
    # True GPD excesses: xi = 0.1, sigma = 50, over threshold u = 2000.
    xi, sigma, u = 0.1, 50.0, 2000.0
    rng = np.random.default_rng(1)
    excess = genpareto.rvs(c=xi, loc=0.0, scale=sigma, size=5000, random_state=rng)

    fit = fit_gpd(u + excess, threshold=u, n_peaks_per_year=50.0)

    assert isinstance(fit, GPDFit)
    assert fit.shape == pytest.approx(xi, abs=0.05)
    assert fit.scale == pytest.approx(sigma, rel=0.05)
    assert fit.threshold == u
    assert fit.n_exceedances == 5000
    assert fit.n_peaks_per_year == 50.0


def test_gev_return_level_matches_fitted_quantile():
    rng = np.random.default_rng(2)
    data = genextreme.rvs(c=0.2, loc=1000.0, scale=100.0, size=500, random_state=rng)

    # One block per year (250-day blocks): the T-year return level is the
    # (1 - 1/T) quantile of the fitted block-maximum distribution.
    fit = fit_gev(data, block_size_days=250)
    for T in [10, 100, 1000]:
        expected = genextreme.ppf(1.0 - 1.0 / T, -fit.shape, fit.loc, fit.scale)
        assert fit.return_level(T) == pytest.approx(expected, rel=1e-12)

    # Sub-year blocks: quantile at 1 - 1/m with m blocks per return period.
    fit_daily = fit_gev(data, block_size_days=1)
    m = 100 * 250.0 / 1.0
    expected = genextreme.ppf(
        1.0 - 1.0 / m, -fit_daily.shape, fit_daily.loc, fit_daily.scale
    )
    assert fit_daily.return_level(100) == pytest.approx(expected, rel=1e-12)


def test_gpd_return_level_round_trip():
    rng = np.random.default_rng(3)
    u, rate = 1500.0, 20.0
    excess = genpareto.rvs(c=0.1, loc=0.0, scale=80.0, size=2000, random_state=rng)

    fit = fit_gpd(u + excess, threshold=u, n_peaks_per_year=rate)
    for T in [50, 100]:
        z = fit.return_level(T)
        # Expected number of exceedances of z within T years is 1:
        # rate * T * P(excess > z - u) == 1.
        p_exceed = genpareto.sf(z - u, c=fit.shape, loc=0.0, scale=fit.scale)
        assert rate * T * p_exceed == pytest.approx(1.0, rel=1e-9)


def test_accepts_series_and_array():
    rng = np.random.default_rng(4)
    bm = genextreme.rvs(c=0.1, loc=500.0, scale=50.0, size=300, random_state=rng)
    fit_arr = fit_gev(bm, block_size_days=25)
    fit_ser = fit_gev(pd.Series(bm), block_size_days=25)
    assert fit_arr == fit_ser

    peaks = 800.0 + genpareto.rvs(
        c=-0.1, loc=0.0, scale=30.0, size=300, random_state=rng
    )
    fit_arr = fit_gpd(peaks, threshold=800.0)
    fit_ser = fit_gpd(pd.Series(peaks), threshold=800.0)
    assert fit_arr == fit_ser


def test_nan_values_are_dropped():
    rng = np.random.default_rng(5)
    bm = genextreme.rvs(c=0.1, loc=500.0, scale=50.0, size=300, random_state=rng)
    with_nan = pd.Series(np.concatenate([bm, [np.nan, np.nan]]))
    assert fit_gev(with_nan, block_size_days=1) == fit_gev(bm, block_size_days=1)


def test_error_cases():
    with pytest.raises(ValueError):
        fit_gev([1.0], block_size_days=1)  # too few points
    with pytest.raises(ValueError):
        fit_gpd([1.0, 2.0, 3.0], threshold=10.0)  # nothing above threshold

    fit = fit_gpd([10.0, 11.0, 12.0, 13.0], threshold=9.0)  # no rate given
    with pytest.raises(ValueError):
        fit.return_level(100)

    gev = GEVFit(shape=0.1, loc=100.0, scale=10.0, block_size_days=250.0)
    with pytest.raises(ValueError):
        gev.return_level(0.5)  # return period shorter than one block


def test_fit_gev_counts_zero_blocks_as_observed():
    # read_BM_S writes 0.0 for a block without an event of the column's size
    # (padding a short row like the engine's own opened-but-empty bucket): such
    # a block was simulated, so it counts in n_blocks_total but not in the fit,
    # and the return level is scaled down by the populated fraction
    rng = np.random.default_rng(17)
    peaks = genextreme.rvs(-0.1, loc=500.0, scale=50.0, size=100, random_state=rng)
    column = pd.Series(np.concatenate([np.zeros(900), peaks]))

    fit = fit_gev(column, block_size_days=1)

    assert fit.n_blocks_used == 100 and fit.n_blocks_total == 1000
    populated_only = fit_gev(peaks, block_size_days=1)
    assert (fit.shape, fit.loc, fit.scale) == (
        populated_only.shape,
        populated_only.loc,
        populated_only.scale,
    )
    assert fit.return_level(100) < populated_only.return_level(100)


# --- diagnostic plots -------------------------------------------------------


def test_analyse_plots_are_exposed_on_package():
    assert pb.analyse.plot_return_level is plot_return_level
    assert pb.analyse.plot_mean_residual_life is plot_mean_residual_life


def test_plot_return_level_and_qq_smoke(tmp_path):
    rng = np.random.default_rng(3)
    maxima = rng.gumbel(100.0, 12.0, 300)
    gev = fit_gev(maxima, block_size_days=1.0)
    peaks = 100.0 + rng.pareto(4.0, 2000) * 20.0
    gpd = fit_gpd(peaks, threshold=110.0, n_peaks_per_year=800.0)

    for name, fit, data in (("gev", gev, maxima), ("gpd", gpd, peaks)):
        plot_return_level(fit, data, save_to=tmp_path / f"rl_{name}.png")
        plot_qq(fit, data, save_to=tmp_path / f"qq_{name}.png")
        assert (tmp_path / f"rl_{name}.png").exists()
        assert (tmp_path / f"qq_{name}.png").exists()

    # the curve alone, without the observations
    plot_return_level(gev, save_to=tmp_path / "rl_curve.png")
    assert (tmp_path / "rl_curve.png").exists()


def test_plot_threshold_diagnostics_smoke(tmp_path):
    rng = np.random.default_rng(4)
    peaks = 100.0 + rng.pareto(4.0, 2000) * 20.0

    plot_mean_residual_life(peaks, save_to=tmp_path / "mrl.png")
    plot_parameter_stability(peaks, save_to=tmp_path / "stability.png")

    assert (tmp_path / "mrl.png").exists()
    assert (tmp_path / "stability.png").exists()


def test_observations_sit_on_the_return_level_curve_they_were_fitted_to():
    # the plotting positions must invert return_level's return period, so the
    # fitted curve passes through the observations of a well-fitting sample
    rng = np.random.default_rng(5)
    maxima = rng.gumbel(100.0, 12.0, 2000)
    fit = fit_gev(maxima, block_size_days=1.0)

    years = _empirical_return_years(fit, maxima.size)
    fitted = np.array([fit.return_level(year) for year in years])
    observed = np.sort(maxima)

    middle = slice(maxima.size // 10, -maxima.size // 10)
    assert np.allclose(fitted[middle], observed[middle], rtol=0.05)


def test_threshold_grid_drops_thresholds_with_too_few_exceedances():
    peaks = np.arange(100.0, 200.0)  # 100 peaks, one per unit

    grid = _threshold_grid(
        peaks, thresholds=[100.0, 150.0, 189.0, 195.0], min_exceedances=10
    )

    assert grid.tolist() == [100.0, 150.0, 189.0]


def test_threshold_grid_refuses_when_no_threshold_is_usable():
    with pytest.raises(ValueError, match="No threshold"):
        _threshold_grid(np.arange(5.0), thresholds=[3.0], min_exceedances=10)
