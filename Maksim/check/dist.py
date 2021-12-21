class DistChecker(object):
    def __init__(self, block_x, block_y, rows, sections):
        self.block_x = block_x
        self.block_y = block_y
        self.rows = rows
        self.sections = sections

    def dist_to_start(self, place):
        ans = 0
        dy = int(abs(self.block_y // 2 + 0.5 - place['block_y']) + 0.5) - 1
        ans += dy * (self.rows + 2)  # расстояние по горизонтали к центру
        ans += (self.block_x - place['block_x']) * (self.sections + 2)  # расстояние по вертикали к низу
        if (place['block_y'] <= self.block_y // 2):
            ans += (self.rows - place['row'])  # расстояние внутри блока
        else:
            ans += place['row']
        ans += (self.sections + 1 - place['section'])
        return ans

    def dist_to_tape(self, place):
        ans = place['section'] + (place['block_x'] - 1) * (self.sections)
        return ans

    def get_dist_2_places(self, p1, p2):
        ans = 0
        if p1['block_x'] != p2['block_x'] or p1['block_y'] != p2['block_y'] or p1['row'] != p2['row']:
            if p1['block_x'] > p2['block_x']: p1, p2 = p2, p1
            ans += (p2['block_x'] - p1['block_x']) * (self.sections + 2)
            ans += min(p1['section'] + p2['section'] + 2, 2 * self.sections - p1['section'] - p2['section'] + 2)
            if p1['block_y'] > p2['block_y']: p1, p2 = p2, p1
            ans += (self.rows - p1['row']) + p2['row']
            ans += (p2['block_y'] - p1['block_y'] - 1) * (self.rows + 2) + 2
            return ans
        else:
            return abs(p1['section'] - p2['section'])

    def get_normal_dist(self, path):
        ans = 0
        ans += self.dist_to_start(path[0][0]) + self.dist_to_start(path[-1][0])
        ans += self.dist_to_tape(path[-1][0]) * 2
        for i in range(len(path) - 1):
            ans += self.get_dist_2_places(path[i][0], path[i + 1][0])
        return ans
