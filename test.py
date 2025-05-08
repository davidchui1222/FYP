from stable_baselines3 import PPO
from hapticMaster import HapticCuttingEnv
import cv2
import imageio
import csv
import time

# Initialize environment
env = HapticCuttingEnv(render_mode="rgb_array", use_advisor=False)
model = PPO.load("./results/best_trials/best_model.zip")

# Reset environment
obs, info = env.reset()

# Prepare to save the sensor data
sensor_data_filename = "./results/best_trials/sensor_data.csv"
with open(sensor_data_filename, mode='w', newline='') as file:
    writer = csv.writer(file)
    # Assuming 'obs' contains sensor data, write header
    writer.writerow(['Time', 'PosX', 'PosY', 'PosZ', 'VelX', 'VelY', 'VelZ', 'ForceX', 'ForceY', 'ForceZ', 'Reward'])

# Time tracking
start_time = time.time()

# For capturing frames for gif
frames = []
for step in range(1000):
    current_time = time.time()
    elapsed_time = current_time - start_time

    action, _states = model.predict(obs, deterministic=True)
    obs, reward, done, truncated, info = env.step(action)
    
    # Log sensor data (assuming 'obs' contains the relevant sensor data)
    with open(sensor_data_filename, mode='a', newline='') as file:
        writer = csv.writer(file)
        # Assuming 'obs' contains sensor data as a list (adjust as per your environment's observation space)
        sensor_data = obs  # Modify if needed based on your env observation structure
        writer.writerow([elapsed_time] + list(sensor_data) + [reward])

    # Render and capture frame
    image = env.render()
    if step % 5 == 0:
        frames.append(image)
    cv2.imshow("image", image)
    cv2.waitKey(1)
    
    # Reset environment if done or truncated
    if done or truncated:
        obs, info = env.reset()
        break

# Uncomment to save result as gif
with imageio.get_writer("./results/best_trials/best_model.gif", mode="I") as writer:
    for idx, frame in enumerate(frames):
        writer.append_data(frame)

# Close the cv2 window after the loop
cv2.destroyAllWindows()
