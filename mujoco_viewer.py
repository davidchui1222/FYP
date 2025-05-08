import mujoco
import csv
import time
import mujoco.viewer
import keyboard

# Load the model from an XML file
with open("mujoco_model/hapticMaster2.xml", "r") as f:
    xml_model = f.read()

assets = {}  # Define your assets if any
model = mujoco.MjModel.from_xml_string(xml_model, assets)
data = mujoco.MjData(model)
horizontal_control_value = 0.4
vertical_control_value = 0.2

# Get the actuator ID for moving the banana in the x-direction
horizontal_force = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_ACTUATOR, "horizontal_force")
vertical_force = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_ACTUATOR, "vertical_force")
if horizontal_force == -1:
    print("horizontal_force actuator not found.")

if vertical_force == -1:
    print("vertical_force actuator not found.")


# Open a CSV file to write the data
with open('manual_simulation_data.csv', 'w', newline='') as csvfile:
    fieldnames = ['Time', 'PosX', 'PosY', 'PosZ', 'VelX', 'VelY', 'VelZ', 'ForceX', 'ForceY', 'ForceZ']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()

    # Launch the viewer
    viewerHandle = mujoco.viewer.launch_passive(model, data)
    
    try:
        # Set a constant velocity for the banana
        while viewerHandle.is_running():

            # left arrow to apply horizontal force
            if keyboard.is_pressed('left'):
                data.ctrl[horizontal_force] = -horizontal_control_value

            if keyboard.is_pressed('right'):
                data.ctrl[horizontal_force] = horizontal_control_value
            # down arrow to apply vertical force
            if keyboard.is_pressed('down'):
                data.ctrl[vertical_force] = -vertical_control_value
            else:
                data.ctrl[vertical_force] = 0

            # 'q' to quit
            if keyboard.is_pressed('q'):
                break

            # Step simulation
            mujoco.mj_step(model, data)

            # Read force sensor data
            pos_sensor_id = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_SENSOR, "EE_pos")
            vel_sensor_id = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_SENSOR, "EE_vel")
            force_sensor_id = mujoco.mj_name2id(model, mujoco.mjtObj.mjOBJ_SENSOR, "EE_force")
            if (pos_sensor_id != -1) and (vel_sensor_id != -1) and (force_sensor_id != 1):
                pos_sensor_adr = model.sensor_adr[pos_sensor_id]
                vel_sensor_adr = model.sensor_adr[vel_sensor_id]
                force_sensor_adr = model.sensor_adr[force_sensor_id]

                pos_sensor_dim = model.sensor_dim[pos_sensor_id]
                vel_sensor_dim = model.sensor_dim[vel_sensor_id]
                force_sensor_dim = model.sensor_dim[force_sensor_id]

                pos = data.sensordata[pos_sensor_adr:pos_sensor_adr + pos_sensor_dim]
                vel = data.sensordata[vel_sensor_adr:vel_sensor_adr + vel_sensor_dim]
                force = data.sensordata[force_sensor_adr:force_sensor_adr + force_sensor_dim]

                current_time = data.time

                writer.writerow({'Time': current_time,
                                 'PosX': pos[0],
                                 'PosY': pos[1],
                                 'PosZ': pos[2],
                                 'VelX': vel[0],
                                 'VelY': vel[1],
                                 'VelZ': vel[2],
                                 'ForceX': force[0], 
                                 'ForceY': force[1], 
                                 'ForceZ': force[2]})

            # Sync viewer
            viewerHandle.sync()

            # Sleep to maintain loop timing
            time.sleep(0.001)
    except Exception as e:
        print(f"Simulation error: {e}")
    finally:
        print("Closing simulation properly.")
