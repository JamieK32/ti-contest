from maix import camera, image

class LineTracker:
    def __init__(self, x=85, y=120, length=25, width=60, offset=0):
        # 红色阈值 (L, A, B)
        self.red = (10, 90, 10, 127, -40, 40)
        
        # 生成8个检测区域 - 反转顺序（适配320x240分辨率）
        self.areas = [(x + length * i + offset, y, length, width) for i in range(8)]
        self.areas.reverse()  # 反转区域顺序
        
        # 优化参数
        self.last_trace = [0, 0, 0, 0, 0, 0, 0, 0]  # 保存上次的检测结果

    def get_trace(self, img):
        """获取循迹数据 - 4点检测版本"""
        trace = [0, 0, 0, 0, 0, 0, 0, 0]
        
        for i, area in enumerate(self.areas):
            x, y, w, h = area
            
            # 4个关键点检测 - 均匀分布
            sample_points = [
                (x + w//5, y + h//2),      # 左1
                (x + 2*w//5, y + h//2),    # 左2
                (x + 3*w//5, y + h//2),    # 右1
                (x + 4*w//5, y + h//2),    # 右2
            ]
            
            red_count = 0
            for px, py in sample_points:
                if px < img.width() and py < img.height():
                    stats = img.get_statistics(roi=(px, py, 5, 5))  # 5x5区域
                    if (self.red[0] < stats.l_mean() < self.red[1] and
                        self.red[2] < stats.a_mean() < self.red[3] and
                        self.red[4] < stats.b_mean() < self.red[5]):
                        red_count += 1
            
            # 4个点中有2个是红色就检测到（约50%准确率）
            if red_count >= 1:
                trace[i] = 1
        
        self.last_trace = trace  # 保存当前结果
        return trace

    def to_number(self, trace):
        """转换为数字"""
        return sum(trace[i] * (2**i) for i in range(8))

    def draw(self, img, trace):
        """绘制检测区域和结果"""
        for i, area in enumerate(self.areas):
            # 根据检测结果改变框的颜色
            color = image.Color.from_rgb(0, 255, 0) if trace[i] else image.Color.from_rgb(0, 0, 0)
            img.draw_rect(area[0], area[1], area[2], area[3], color=color)
            img.draw_string(area[0] + 6, area[1] + 1, str(trace[i]), color=image.Color.from_rgb(255, 255, 255))