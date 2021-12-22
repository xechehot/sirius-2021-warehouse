import pytest

from check.dist import DistChecker


def test_simple_sku_block():
    dist_checker = DistChecker(block_x=1,
                               block_y=2,
                               rows=2,
                               sections=1)
    path = [
        dict(
            block_x=1,
            block_y=1,
            row=1,
            section=1
        )
    ]
    ans = dist_checker.get_normal_dist(path)
    assert ans == 6


def test_one_sku_block():
    dist_checker = DistChecker(block_x=1,
                               block_y=2,
                               rows=20,
                               sections=10)
    path = [
        dict(
            block_x=1,
            block_y=1,
            row=10,
            section=2
        )
    ]
    ans = dist_checker.get_normal_dist(path)
    assert ans == 42


def test_two_sku_block():
    dist_checker = DistChecker(block_x=1,
                               block_y=2,
                               rows=20,
                               sections=10)
    path = [
        dict(
            block_x=1,
            block_y=1,
            row=10,
            section=9
        ),
        dict(
            block_x=1,
            block_y=1,
            row=10,
            section=2
        ),
    ]
    dist_2_point = dist_checker.get_dist_2_places(path[0], path[1])
    assert dist_2_point == 7
    ans = dist_checker.get_normal_dist(path)
    assert ans == 42


def test_onw_sku_2_blocks():
    dist_checker = DistChecker(block_x=2,
                               block_y=2,
                               rows=20,
                               sections=10)
    path = [
        dict(
            block_x=2,
            block_y=1,
            row=10,
            section=2
        )
    ]
    ans = dist_checker.get_normal_dist(path)
    assert ans == 62