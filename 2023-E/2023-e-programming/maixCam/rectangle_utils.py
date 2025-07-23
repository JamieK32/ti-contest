import math
from maix import image


class RectangleUtils:
    """矩形绘制和路径生成工具类"""
    
    def __init__(self, default_step=20, default_color=None):
        """
        初始化矩形工具
        
        参数:
        default_step: 默认步长
        default_color: 默认颜色
        """
        self.default_step = default_step
        self.default_color = default_color or image.COLOR_WHITE
    
    def set_step(self, step):
        """
        设置默认步长
        
        参数:
        step: 新的步长值
        """
        self.default_step = step
    
    def set_color(self, color):
        """
        设置默认颜色
        
        参数:
        color: 新的颜色值
        """
        self.default_color = color
    
    def get_step(self):
        """
        获取当前默认步长
        
        返回:
        int: 当前默认步长
        """
        return self.default_step
    
    def get_color(self):
        """
        获取当前默认颜色
        
        返回:
        颜色值: 当前默认颜色
        """
        return self.default_color
        
    def generate_points_on_edge(self, start_point, end_point, step=None):
        """
        在两个点之间生成等间距、包含起点和终点的离散点列表（收尾相连）
        
        参数:
        start_point (tuple): 起始点 (x, y)
        end_point (tuple): 结束点 (x, y)
        step (float): 点之间的步长
        
        返回:
        List[tuple]: 离散点列表（包含起点和终点，且无重复）
        """
        if step is None:
            step = self.default_step
            
        x1, y1 = start_point
        x2, y2 = end_point
        
        # 计算两点之间的欧几里得距离
        distance = math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)
        
        # 距离太小直接返回两个端点
        if distance <= step:
            return [start_point, end_point] if start_point != end_point else [start_point]
        
        # 计算步数，确保包含最后一个点
        num_steps = int(distance / step)
        points = []
        
        for i in range(num_steps + 1):
            t = i / num_steps
            x = x1 + t * (x2 - x1)
            y = y1 + t * (y2 - y1)
            points.append((int(round(x)), int(round(y))))
        
        # 添加 end_point 做收尾连接（若不同于最后一个点）
        if points[-1] != end_point:
            points.append(end_point)
        
        # 去重（避免连续重复的点）
        result = []
        for pt in points:
            if not result or pt != result[-1]:
                result.append(pt)
        
        return result
    
    def get_rectangle_center(self, rectangle_points):
        """
        根据对角线交点计算矩形中心点
        
        参数:
        rectangle_points: 矩形四个顶点坐标列表
        
        返回:
        tuple: 矩形中心点坐标 (x, y)
        """
        x1, y1 = rectangle_points[0]
        x2, y2 = rectangle_points[2]
        
        return (x1 + x2) // 2, (y1 + y2) // 2
    
    def draw_rectangle(self, img, points, color=None):
        """
        仅绘制矩形，不返回路径点
        
        参数:
        img: 图像对象
        points: 矩形四个顶点坐标列表
        color: 绘制颜色
        """
        if color is None:
            color = self.default_color
            
        for i in range(4):
            start, end = points[i], points[(i + 1) % 4]
            img.draw_line(start[0], start[1], end[0], end[1], color=color, thickness=2)

    def generate_rectangle_points(self, points, step=None):
        """
        生成矩形边缘的路径点
        
        参数:
        points: 矩形四个顶点坐标列表
        step: 步长（可选，默认使用类的default_step）
        
        返回:
        List[tuple]: 矩形边缘的所有路径点
        """
        if step is None:
            step = self.default_step
            
        path_points = []
        
        # 遍历矩形的四条边
        for i in range(4):
            start = points[i]
            end = points[(i + 1) % 4]
            
            # 生成这条边上的点
            edge_points = self.generate_points_on_edge(start, end, step)
            
            # 除了第一条边，其他边要去掉起始点以避免重复
            if i == 0:
                path_points.extend(edge_points)
            else:
                # 跳过起始点，因为它和上一条边的终点重复
                path_points.extend(edge_points[1:])
        
        # 确保路径是闭合的（最后一个点应该接近第一个点）
        if path_points and path_points[-1] != path_points[0]:
            # 可以选择是否添加回到起点的连接，这里不添加以避免重复
            pass
            
        return path_points