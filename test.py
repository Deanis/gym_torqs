#!/usr/bin/python3.6
from gym_torcs import TorcsEnv
from sample_agent import Agent
import numpy as np
import os
import matplotlib.pyplot as plt

# Some params
vision = True
max_eps = 10
max_steps = 1000
reward = 0
done = False
step = 0

gear_change = False
rendering = True
lap_limiter = 1

# 10 Fixed Sparsed Config 2m not too much bots in corners
race_config_path = os.path.dirname(os.path.abspath(__file__)) + \
    "/raceconfig/agent_practice_test.xml"

# Generate a Torcs environment
env = TorcsEnv(vision=vision,
    throttle=True,
    gear_change=False,
	race_config_path=race_config_path,
    rendering=rendering,
	lap_limiter = lap_limiter)

agent = Agent(2)  # steering and accel random
ob = env.reset()

print("TORCS Experiment Start.")
try:
    for i in range(max_eps):
        print("Episode : " + str(i))

        total_reward = 0.
        for t in range(max_steps):
            action = agent.act(ob, reward, done, vision)

            ob, reward, done, _ = env.step(action)
            total_reward += reward

            # Debug begin
            # print(ob)
            # input()
            
            # Prepocess the obs
            focus, speedX, speedY, speedZ, opponents, rpm, track, wheelSpinVel, vision = ob

            img = np.ndarray((64,64,3))
            for i in range(3):
                img[:, :, i] = 255 - vision[:, i].reshape((64, 64))

            plt.imshow(img, origin='lower')
            plt.draw()
            plt.pause(0.001)
            # Debug end

            step += 1
            if done:
                break

                print("TOTAL REWARD @ " + str(i) +" -th Episode  :  " + str(total_reward))
                print("Total Step: " + str(step))
                print("")

except Exception as e:
    print( e.systrace())

finally:
    env.end()

# env.end()  # This is for shutting down TORCS
print("Finish.")
