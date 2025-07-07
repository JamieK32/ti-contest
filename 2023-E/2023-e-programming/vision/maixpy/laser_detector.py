from maix import camera, display, app, image

class LaserDetector:
    def __init__(self, thresholds=None, area_range=(5, 500)):
        self.red_thresholds = thresholds or [
            [[50, 100, 3, 127, 0, 127]],
            [[17, 37, 5, 127, -4, 127]],
            [[40, 90, 10, 80, 5, 70]],
            [[0, 32, 8, 88, -3, 127]],
            [[23, 36, 4, 88, -4, 51]]
        ]
        self.current_threshold_index = 0
        self.min_area, self.max_area = area_range
        self.area_threshold = 2
        self.pixels_threshold = 2
    
    def lightweight_filter(self, blobs):
        if not blobs:
            return None
        
        valid_blobs = [b for b in blobs if self.min_area <= b[4] <= self.max_area]
        
        if not valid_blobs:
            return None
        
        return max(valid_blobs, key=lambda b: b[4])
    
    def detect(self, img, draw=True):
        for i in range(len(self.red_thresholds)):
            current_threshold = self.red_thresholds[self.current_threshold_index]
            blobs = img.find_blobs(current_threshold, pixels_threshold=self.pixels_threshold)
            
            best_blob = self.lightweight_filter(blobs)
            
            if best_blob:
                raw_cx = best_blob[0] + best_blob[2] // 2
                raw_cy = best_blob[1] + best_blob[3] // 2
                
                if draw:
                    color = image.Color.from_rgb(0, 255, 255)
                    img.draw_line(raw_cx - 10, raw_cy, raw_cx + 10, raw_cy, color=color, thickness=2)
                    img.draw_line(raw_cx, raw_cy - 10, raw_cx, raw_cy + 10, color=color, thickness=2)
                    img.draw_circle(raw_cx, raw_cy, 3, color=color, thickness=2)
                
                return raw_cx, raw_cy
            
            self.current_threshold_index = (self.current_threshold_index + 1) % len(self.red_thresholds)
        
        return -1, -1
    
    def set_color_threshold(self, thresholds):
        self.red_thresholds = thresholds
    
    def set_area_range(self, min_area, max_area):
        self.min_area = min_area
        self.max_area = max_area