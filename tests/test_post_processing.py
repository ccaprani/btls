"""
Tests for pybtls.post_processing extreme value fitting (fit_gev / fit_gpd).
"""

import numpy as np
import pandas as pd
import pytest
from scipy.stats import genextreme, genpareto

import pybtls as pb
from pybtls.post_processing import fit_gev, fit_gpd, GEVFit, GPDFit


def test_module_is_exposed_on_package():
    assert pb.post_processing.fit_gev is fit_gev
    assert pb.post_processing.fit_gpd is fit_gpd


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
