from maix import camera, display, app, image

class LaserDetector:
    def __init__(self, thresholds=None, area_range=(5, 500)):
        self.red_thresholds = thresholds or [
            [[50, 100, 3, 127, 0, 127]],
            [(45, 86, 16, 123, -8, 127)],
            [[17, 37, 5, 127, -4, 127]],
            [[40, 90, 10, 80, 5, 70]],
            [[0, 32, 8, 88, -3, 127]],
            [[23, 36, 4, 88, -4, 51]],
            [(5, 75, 11, 65, -128, 127)]
        ]
        self.current_threshold_index = 0
        self.min_area, self.max_area = area_range
        self.area_threshold = 2
        self.pixels_threshold = 2
        # Cache color object
        self.crosshair_color = image.Color.from_rgb(0, 255, 255)

    def find_best_blob(self, blobs):
        """Find the largest valid blob in one pass"""
        if not blobs:
            return None
        
        best_blob = None
        max_area = 0
        
        for blob in blobs:
            area = blob[4]
            if self.min_area <= area <= self.max_area and area > max_area:
                max_area = area
                best_blob = blob
                
        return best_blob

    def detect(self, img, draw=True):
        # Try current threshold first, then cycle through others
        thresholds_to_try = len(self.red_thresholds)
        
        for i in range(thresholds_to_try):
            threshold_idx = (self.current_threshold_index + i) % thresholds_to_try
            current_threshold = self.red_thresholds[threshold_idx]
            
            blobs = img.find_blobs(current_threshold, pixels_threshold=self.pixels_threshold)
            best_blob = self.find_best_blob(blobs)
            
            if best_blob:
                # Update current threshold for next time
                self.current_threshold_index = threshold_idx
                
                # Calculate center
                cx = best_blob[0] + best_blob[2] // 2
                cy = best_blob[1] + best_blob[3] // 2
                
                if draw:
                    # Draw crosshair and circle
                    img.draw_line(cx - 10, cy, cx + 10, cy, color=self.crosshair_color, thickness=2)
                    img.draw_line(cx, cy - 10, cx, cy + 10, color=self.crosshair_color, thickness=2)
                    img.draw_circle(cx, cy, 3, color=self.crosshair_color, thickness=2)
                
                return cx, cy
        
        # No detection found, move to next threshold for next frame
        self.current_threshold_index = (self.current_threshold_index + 1) % len(self.red_thresholds)
        return -1, -1

    def set_color_threshold(self, thresholds):
        self.red_thresholds = thresholds

    def set_area_range(self, min_area, max_area):
        self.min_area = min_area
        self.max_area = max_area