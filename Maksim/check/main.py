import json

import click

from dist import DistChecker


def load_file(path):
    with open(path, 'r') as f:
         return json.load(f)

@click.command()
@click.option("--sample", help="Входной файл")
@click.option("--output", help="Выходной файл")
def check_dist(sample, output):
    sample_data = load_file(sample)
    output_data = load_file(output)
    wh_meta = sample_data['warehouse']['meta']
    checker = DistChecker(block_x=wh_meta['block_x'],
                          block_y=wh_meta['block_y'],
                          rows=wh_meta['rows'],
                          sections=wh_meta['sections'])
    total_dist = checker.get_batches_dist(output_data)
    print(f'Total dist: {total_dist}')
    total_items = checker.get_total_items(output_data)
    print(f'Total items: {total_items}')
    print(f'Avg dist per item: {total_dist/total_items}')


if __name__ == '__main__':
    check_dist()
