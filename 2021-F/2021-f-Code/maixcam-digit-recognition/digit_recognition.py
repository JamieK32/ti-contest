from maix import image, nn

class ObjectDetector:
    def __init__(self, model_path="/root/model_225498.mud", conf_th=0.8, iou_th=0.45):
        self.detector = nn.YOLOv5(model=model_path)
        self.conf_th = conf_th
        self.iou_th = iou_th
        self.red_center_x = None
        
        # 缓存检测结果
        self.last_result = None
        self.detected_number = None
        self.position = None
        
    def find_red_center_once(self, img):
        """只检测一次红线中心"""
        if self.red_center_x is not None:
            return self.red_center_x
            
        red_counts = [0] * img.width()
        mid_y = img.height() // 2
        
        for y in range(mid_y - 20, mid_y + 20, 5):
            for x in range(0, img.width(), 3):
                try:
                    pixel = img.get_pixel(x, y)
                    if isinstance(pixel, (list, tuple)):
                        r, g, b = pixel[0], pixel[1], pixel[2]
                    else:
                        r = (pixel >> 16) & 0xFF
                        g = (pixel >> 8) & 0xFF
                        b = pixel & 0xFF
                        
                    if r > 150 and g < 100 and b < 100:
                        red_counts[x] += 1
                except:
                    continue
        
        max_count = max(red_counts) if red_counts else 0
        self.red_center_x = red_counts.index(max_count) if max_count > 0 else img.width() // 2
        return self.red_center_x
        
    def detect_numbers(self, img):
        """检测数字并返回数字_方位格式"""
        objs = self.detector.detect(img, conf_th=self.conf_th, iou_th=self.iou_th)
        
        if not objs:
            self.last_result = None
            self.detected_number = None
            self.position = None
            return None
            
        obj = objs[0]
        red_center_x = self.find_red_center_once(img)
        obj_center_x = obj.x + obj.w // 2
        
        # 更新类成员
        self.detected_number = self.detector.labels[obj.class_id]
        self.position = 0x01 if obj_center_x < red_center_x else 0x02
        self.last_result = f"{self.detected_number}_{self.position}"
        
        # 绘制
        img.draw_rect(obj.x, obj.y, obj.w, obj.h, color=image.COLOR_RED)
        position_text = "LEFT" if self.position == 0x01 else "RIGHT"
        img.draw_string(obj.x, obj.y, f"{self.detected_number}_{position_text}", color=image.COLOR_RED)
        
        return self.last_result
    
    def get_number(self):
        """获取检测到的数字"""
        return self.detected_number
    
    def get_position(self):
        """获取检测到的方位"""
        return self.position
    
    def get_result(self):
        """获取完整结果"""
        return self.last_result