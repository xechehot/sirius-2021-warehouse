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


def test_onw_sku_2_far_blocks():
    dist_checker = DistChecker(block_x=2,
                               block_y=2,
                               rows=6,
                               sections=5)
    path = [
        dict(
            block_x=1,
            block_y=1,
            row=2,
            section=1
        ),
        dict(
            block_x=1,
            block_y=2,
            row=1,
            section=1
        )
    ]
    ans = dist_checker.get_dist_2_places(path[0], path[1])
    assert ans == 8


def test_batch_size_dist():
    dist_checker = DistChecker(block_x=2,
                               block_y=2,
                               rows=20,
                               sections=10)
    result = {
        "batches": [
            {
                "buckets": [
                    {
                        "items": [
                            {
                                "block_x": 1,
                                "block_y": 2,
                                "cnt": 2,
                                "floor": 1,
                                "id": 68542596,
                                "row": 19,
                                "section": 6
                            },
                            {
                                "block_x": 1,
                                "block_y": 2,
                                "cnt": 1,
                                "floor": 1,
                                "id": 81522366,
                                "row": 18,
                                "section": 7
                            }
                        ]
                    }
                ]
            }
        ]
    }
    assert dist_checker.get_batch_dist(result['batches'][0]) == 111
    assert dist_checker.get_batches_dist(result) == 111
    assert dist_checker.get_total_items(result) == 3