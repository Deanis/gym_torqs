#!/usr/bin/env python3
### From A2C
from baselines import logger
#Customized OpenAI Baselines functions
from cmd_util import make_torcs_env, torcs_arg_parser
from baselines.common.vec_env.vec_frame_stack import VecFrameStack
from a2c import learn
from baselines.ppo2.policies import CnnPolicy, LstmPolicy, LnLstmPolicy
from gym_torcs_wrpd import TorcsEnv
### From Gym Torcs Wrapped
# from gym_torcs_wrpd import TorcsEnv
# from sample_agent import Agent
import numpy as np

def train( num_timesteps, seed, policy, lrschedule, num_env):
    if policy == 'cnn':
        policy_fn = CnnPolicy
    elif policy == 'lstm':
        policy_fn = LstmPolicy
    elif policy == 'lnlstm':
        policy_fn = LnLstmPolic

    #Torqs Env parameters
    vision, throttle, gear_change = True, False, False

    env = VecFrameStack(make_torcs_env( num_env, seed, \
        vision=vision, throttle=throttle, gear_change=gear_change), 4)
    # env = TorcsEnv( vision=vision, throttle=throttle, gear_change=gear_change)

    learn(policy_fn, env, seed, total_timesteps=int(num_timesteps * 1.1), lrschedule=lrschedule)
    env.close()

def main():
    parser = torcs_arg_parser()
    parser.add_argument('--policy', help='Policy architecture', choices=['cnn', 'lstm', 'lnlstm'], default='lnlstm')
    parser.add_argument('--lrschedule', help='Learning rate schedule', choices=['constant', 'linear'], default='constant')
    args = parser.parse_args()
    logger.configure()
    train( num_timesteps=args.num_timesteps, seed=args.seed,
        policy=args.policy, lrschedule=args.lrschedule, num_env=1)

if __name__ == '__main__':
    main()

def torqs_test():
    #Torcs multiple instance test
    vision = False
    vision1 = False
    episode_count = 1
    episode_count1 = 1
    max_steps = 10000
    reward = 0
    reward1 = 0
    done = False
    done1 = False
    step = 0
    step1 = 0

    # Generate a Torcs environment
    env = TorcsEnv(vision=vision, throttle=False)
    env1= TorcsEnv(vision=True, throttle=False)

    agent = Agent(1)  # steering only
    agent1 = Agent( 1)

    print("TORCS Experiment Start.")
    for i in range(episode_count):
        print("Episode : " + str(i))

        if np.mod(i, 3) == 0:
            # Sometimes you need to relaunch TORCS because of the memory leak error
            ob = env.reset(relaunch=True)
            ob1 = ob = env1.reset(relaunch=True)
        else:
            ob = env.reset()
            ob1 = env1.reset()

        total_reward = 0.
        total_reward1 = 0.
        for j in range(max_steps):
            action = agent.act(ob, reward, done, vision)
            action1 = agent1.act(ob1, reward, done, vision)

            ob, reward, done, _ = env.step(action)
            ob1, reward1, done1, _ = env1.step( action1)
            #print(ob)
            total_reward += reward
            total_reward1 += reward1

            step += 1
            step1 += 1
            if done and done1:
                break

        print("TOTAL REWARD @ " + str(i) +" -th Episode  :  " + str(total_reward))
        print("Total Step: " + str(step))
        print("")

        print("TOTAL REWARD1 @ " + str(i) +" -th Episode  :  " + str(total_reward1))
        print("Total Step: " + str(step1))
        print("")

    env.end()  # This is for shutting down TORCS
    print("Finish.")

# torqs_test()
