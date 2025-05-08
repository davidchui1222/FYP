import numpy as np
from gymnasium import utils
from gymnasium.envs.mujoco import MujocoEnv
from gymnasium.spaces import Box
import os
from advisor.language_policy_advisor import LanguagePolicyAdvisor

class HapticCuttingEnv(MujocoEnv, utils.EzPickle):
    metadata = {
        "render_modes": ["human", "rgb_array", "depth_array"],
        "render_fps": 200,
    }

    def __init__(self, episode_len=1000, use_advisor=False, **kwargs):
        utils.EzPickle.__init__(self, **kwargs)
        self.use_advisor = use_advisor
        
        observation_space = Box(low=-np.inf, high=np.inf, shape=(9,), dtype=np.float64)
        MujocoEnv.__init__(
            self,
            os.path.abspath("mujoco_model/hapticMaster2.xml"),
            5,
            observation_space=observation_space,
            **kwargs
        )

        self.language_policy_advisor = LanguagePolicyAdvisor()
        self.record = ""
        
        self.actuator_indices = [0, 1, 2]
        self.action_space = Box(low=-1.0, high=1.0, shape=(len(self.actuator_indices),), dtype=np.float64)
        self.episode_len = episode_len
        self.step_number = 0

        self.banana_found = False
        self.banana_posX = 0
        self.banana_posZ = 0

        self.distanceReached = False
        self.distanceX = 0
        self.distanceZ = 0

        self.force_magnitude = 0
        self.force_ema = 0
        self.ema_alpha = 0.2  # smoothing factor, between 0 and 1

    def step(self, action):
        full_action = np.zeros(self.model.nu)
        full_action[self.actuator_indices] = action

        self.do_simulation(full_action, self.frame_skip)
        self.step_number += 1

        obs = self._get_obs()
        position_feedback = obs[0:3]
        velocity_feedback = obs[3:6]
        force_feedback = obs[6:9]

        self.force_magnitude = np.linalg.norm(force_feedback)
        self.force_ema = (1 - self.ema_alpha) * self.force_ema + self.ema_alpha * self.force_magnitude

        reward = 0

        # ============= Language Advisor ==============
        if self.use_advisor:
            # Context-driven reward via a language advisor
            state_msg = f"Step {self.step_number}:\n"
            position_msg = f"Current positions: X={position_feedback[0]}, Y={position_feedback[1]}, Z={position_feedback[2]}.\n"
            velocity_msg = f"Current velocities: X={velocity_feedback[0]}, Y={velocity_feedback[1]}, Z={velocity_feedback[2]}.\n"
            force_msg = f"Current forces: X={force_feedback[0]}, Y={force_feedback[1]}, Z={force_feedback[2]}.\n"

            sentence = state_msg + position_msg + velocity_msg + force_msg
            self.record = self.record + sentence

            reward -= 1000 * abs(velocity_feedback[1])
        # =============== Change State: Banana Found ================
        if (not self.banana_found) and (self.force_ema >= 1):
            reward += 100
            print("Banana Found!")
            self.banana_found = True 
            self.banana_posX = position_feedback[0]
            self.banana_posZ = position_feedback[2]
        
        # ============= State: Banana not found ====================
        if not self.banana_found:
            reward -= 10
            
            # Reward downward movement / Penalise upward movement
            reward += np.clip(10 * (-velocity_feedback[2] / 0.1), 0, 10)

            # Reward forward movement / Penalise backward movement
            reward += np.clip(10 * np.exp(-((velocity_feedback[0] + 0.05) ** 2) / (2 * 0.1**2)), 0, 10)

            if abs(velocity_feedback[0]) > 0.05:
                reward -= 50

        # ============= State: Banana found =======================
        if self.banana_found:
            self.distanceX = (self.banana_posX - position_feedback[0])
            self.distanceZ = (position_feedback[2] - self.banana_posZ)
            reward += np.clip(50 * np.exp(-((self.distanceX - 0.3) ** 2) / (2 * 0.05**2)), 0, 50)

            #print(f"{self.distanceX:.4f}, {self.distanceZ:.4f}, {velocity_feedback[0]:.4f}")

            # ============= State: Within Distance ===============
            if self.distanceX <= 0.2 and not self.distanceReached:

                # Reward forward movement / Penalise backward movement
                reward += np.clip(10 * np.exp(-((force_feedback[0] + 0.4) ** 2) / (2 * 0.1**2)), 0, 10)
                reward += np.clip(100 * -velocity_feedback[0], 0, 10)
                
            # ============= State: Out of Distance ==============
            if self.distanceX > 0.2:
                print("============== Distance Reached! ==============")
                self.distanceReached = True
                print(f"{self.distanceX:.4f}, {self.distanceZ:.4f}, {velocity_feedback[2]:.4f}")
                reward += 20
                # Reward upward movement / Penalise downward movement
                reward += np.clip(50 * (velocity_feedback[2] / 0.1), 0, 50)
                reward += np.clip(50 * np.exp(-((self.distanceZ - 0.1) ** 2) / (2 * 0.05**2)), 0, 50)
                if self.distanceZ > -0.1:
                    reward += 100

        done = (not np.isfinite(obs).all()) or (self.distanceX > 0.3)
        truncated = self.step_number >= self.episode_len or (self.distanceX > 0.3)


        if (done or truncated) and self.use_advisor:
            advisor_reward = self.language_policy_advisor.query_advisor(self.record)
            advisor_reward *= 10
            if advisor_reward <= 80:
                reward -= advisor_reward
            else:
                reward += advisor_reward

        return obs, reward, done, truncated, {}

    def reset_model(self):
        self.step_number = 0
        qpos = self.init_qpos + self.np_random.uniform(size=self.model.nq, low=-0.01, high=0.01)
        qvel = self.init_qvel + self.np_random.uniform(size=self.model.nv, low=-0.01, high=0.01)
        self.set_state(qpos, qvel)
        self.banana_found = False
        self.banana_posX = 0
        self.banana_posZ = 0

        self.distanceReached = False
        self.distanceX = 0
        self.distanceZ = 0

        self.force_magnitude = 0
        self.force_ema = 0
        self.ema_alpha = 0.2  # smoothing factor, between 0 and 1

        return self._get_obs()

    def _get_obs(self):
        return np.concatenate((
            self.data.sensordata[:3],  # EE position
            self.data.sensordata[3:6], # EE velocity
            self.data.sensordata[6:9], # Force feedback
        ), axis=0)
