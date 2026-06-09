import serial
import numpy as np
import cv2  # Add this for saving
import os
import time

# --- Setup ---
ser = serial.Serial('COM8', 115200)
ser.reset_input_buffer()

# Create folder if it doesn't exist
save_path = "thermal_dataset/images"
save_frames="thermal_dataset/frames_images"
if not os.path.exists(save_path):
    os.makedirs(save_path)
if not os.path.exists(save_frames):
    os.makedirs(save_frames)

c=0
try:
    while True:
        line = ser.readline().decode(errors='ignore').strip()
        if not line or "Failed" in line:
            continue

        # safer parsing
        try:
            data = [float(x) for x in line.split(',') if x.strip()]
        except:
            continue

        if len(data) == 768:
            # 1. Process Data
            frame = np.array(data).reshape(24, 32)
            frame = np.flipud(frame)

            timestamp = int(time.time() * 1000)  # ms precision

            # =========================
            # 2. SAVE RAW DATA (IMPORTANT)
            # =========================
            np.save(f"{save_frames}/frame_{timestamp}_{c}.npy", frame)

            # =========================
            # 3. DYNAMIC NORMALIZATION
            # =========================
            min_val = np.min(frame)
            max_val = np.max(frame)

            if max_val - min_val < 1e-6:
                continue  # skip useless frame

            norm_frame = ((frame - min_val) / (max_val - min_val) * 255).astype(np.uint8)

            # =========================
            # 4. OPTIONAL RESIZE (for CNN)
            # =========================
            cnn_frame = cv2.resize(norm_frame, (64, 64))

            # =========================
            # 5. SAVE IMAGE (CNN input)
            # =========================
            filename = f"{save_path}/frame_{timestamp}_{c}.png"
            cv2.imwrite(filename, cnn_frame)

            # =========================
            # 6. DISPLAY (for you, not ML)
            # =========================
            display_frame = cv2.resize(norm_frame, (640, 480), interpolation=cv2.INTER_CUBIC)
            display_frame = cv2.applyColorMap(display_frame, cv2.COLORMAP_INFERNO)
            cv2.imshow('Thermal Live Feed', display_frame)

            print(f"Saved: {filename}")

            c += 1
            if c == 10:
                break

            # small delay (prevents duplicate frames spam)
            time.sleep(0.05)

except Exception as e:
    print(f"Error: {e}")

finally:
    ser.close()
    cv2.destroyAllWindows()