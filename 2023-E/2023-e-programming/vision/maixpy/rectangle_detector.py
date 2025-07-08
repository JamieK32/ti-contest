from maix import image, camera, display, app
import cv2
import numpy as np

class RectangleDetector:
    """矩形检测器 - 找到最大矩形并向内缩进"""
    
    def __init__(self, cam, disp, inset_pixels=5):
        self.cam = cam
        self.disp = disp
        self.kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (3, 3))
        self.inset_pixels = inset_pixels  # 内缩像素数，可调节

    def preprocess(self, img_raw):
        """图像预处理"""
        img_gray = cv2.cvtColor(img_raw, cv2.COLOR_BGR2GRAY)
        
        # 增强对比度
        clahe = cv2.createCLAHE(clipLimit=2.0, tileGridSize=(8, 8))
        img_enhanced = clahe.apply(img_gray)
        
        # 双边滤波去噪
        img_filtered = cv2.bilateralFilter(img_enhanced, 9, 75, 75)
        
        # 形态学操作
        img_morphed = cv2.morphologyEx(img_filtered, cv2.MORPH_CLOSE, self.kernel)
        
        # 边缘检测 - 调整参数使其更敏感
        edges = cv2.Canny(img_morphed, 30, 100)
        
        return edges

    def order_points(self, points):
        """排序点：左上、右上、右下、左下（顺时针）"""
        points = np.array(points, dtype=np.float32)
        s = points.sum(axis=1)
        diff = np.diff(points, axis=1)
        
        ordered = np.zeros((4, 2), dtype=np.float32)
        ordered[0] = points[np.argmin(s)]      # 左上
        ordered[1] = points[np.argmax(diff)]   # 右上  
        ordered[2] = points[np.argmax(s)]      # 右下
        ordered[3] = points[np.argmin(diff)]   # 左下
        
        return [(int(x), int(y)) for x, y in ordered]

    def inset_rectangle(self, points, inset):
        """将矩形向内缩进指定像素数"""
        if len(points) != 4:
            return None
        
        # points顺序：左上、右上、右下、左下
        top_left, top_right, bottom_right, bottom_left = points
        
        # 计算向内缩进后的角点
        inset_points = [
            (top_left[0] + inset, top_left[1] + inset),         # 左上：右下移
            (top_right[0] + inset, top_right[1] - inset),       # 右上：左下移
            (bottom_right[0] - inset, bottom_right[1] - inset), # 右下：左上移
            (bottom_left[0] - inset, bottom_left[1] + inset)    # 左下：右上移
        ]
        
        return inset_points

    def get_inset_corners(self):
        """获取最大矩形内缩后的角点"""
        img = self.cam.read()
        img_raw = image.image2cv(img, copy=False)
        edges = self.preprocess(img_raw)
        
        # 显示边缘检测结果用于调试
        # edges_show = image.cv2image(edges, copy=False)
        # self.disp.show(edges_show)
        
        contours, hierarchy = cv2.findContours(edges, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
        
        rectangles = []
        
        # 找到所有矩形
        for contour in contours:
            area = cv2.contourArea(contour)
            if area < 200:  # 最小面积阈值
                continue
            
            # 调整epsilon参数，使轮廓近似更宽松
            epsilon = 0.05 * cv2.arcLength(contour, True)
            approx = cv2.approxPolyDP(contour, epsilon, True)
            
            # 更宽松的条件：允许4-6个顶点的近似四边形
            if 4 <= len(approx) <= 6:
                # 如果是5或6个点，取最外围的4个点
                if len(approx) > 4:
                    # 计算轮廓的外接矩形
                    rect = cv2.minAreaRect(contour)
                    box = cv2.boxPoints(rect)
                    approx = np.array([[point] for point in box], dtype=np.int32)
                
                points = [(int(point[0][0]), int(point[0][1])) for point in approx]
                points = self.order_points(points)
                
                rectangles.append({
                    'points': points,
                    'area': area
                })
        
        print(f"找到 {len(rectangles)} 个矩形候选")
        
        if len(rectangles) == 0:
            print("未检测到任何矩形")
            # 显示原图
            img_show = image.cv2image(img_raw, copy=False)
            self.disp.show(img_show)
            return None
        
        # 按面积排序，获取最大矩形
        rectangles.sort(key=lambda x: x['area'], reverse=True)
        max_rect = rectangles[0]  # 面积最大的矩形
        
        print(f"最大矩形面积: {max_rect['area']}")
        print(f"内缩像素数: {self.inset_pixels}")
        
        # 绘制所有检测到的矩形用于调试
        for i, rect in enumerate(rectangles):
            if i == 0:  # 最大矩形用蓝色
                color = (255, 0, 0)
                thickness = 2
            else:  # 其他矩形用灰色
                color = (128, 128, 128)
                thickness = 1
            
            cv2.polylines(img_raw, [np.array(rect['points'])], True, color, thickness)
            # 在矩形中心标注面积
            center = np.mean(rect['points'], axis=0).astype(int)
            cv2.putText(img_raw, f"{int(rect['area'])}", tuple(center), 
                       cv2.FONT_HERSHEY_SIMPLEX, 0.4, color, 1)
        
        # 获取最大矩形的角点并内缩
        max_points = max_rect['points']
        inset_corners = self.inset_rectangle(max_points, self.inset_pixels)
        
        if inset_corners:
            # 重新排列为1432顺序：左上、左下、右下、右上
            reordered_points = [
                inset_corners[0],  # 左上
                inset_corners[3],  # 左下 
                inset_corners[2],  # 右下
                inset_corners[1]   # 右上
            ]
            
            # 绘制内缩矩形用红色
            cv2.polylines(img_raw, [np.array(inset_corners)], True, (0, 0, 255), 3)
            
            # 绘制角点和编号
            for i, (x, y) in enumerate(reordered_points):
                cv2.circle(img_raw, (x, y), 6, (0, 255, 255), -1)  # 黄色角点
                cv2.putText(img_raw, str(i), (x+10, y-10), 
                           cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
            
            # 在图像上标注说明
            cv2.putText(img_raw, f"Blue: Max, Red: Inset(-{self.inset_pixels}px)", 
                       (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            
            # 显示结果图像
            img_show = image.cv2image(img_raw, copy=False)
            self.disp.show(img_show)
            
            return reordered_points
        
        # 显示原图（内缩失败）
        img_show = image.cv2image(img_raw, copy=False)
        self.disp.show(img_show)
        return None

    def set_inset_pixels(self, pixels):
        """设置内缩像素数"""
        self.inset_pixels = pixels
        print(f"内缩像素数已设置为: {pixels}")

    def detect_once(self):
        """单次检测并返回内缩后的角点"""
        inset_corners = self.get_inset_corners()
        if inset_corners:
            print(f"检测成功! 内缩角点(1432顺序): {inset_corners}")
            return inset_corners
        else:
            print("未能检测到内缩角点")
            return None