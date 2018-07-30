#! /usr/bin/python3
'''
Binh Nguyen, July 1, 2018
A simulation using Python3 to model algal biomass production:
input: using solar profile from Phoenix, AZ, USA
constant: marked as UPPER_CASE below
key assumptions: 
1. Light intensity: calculated as the volume average based the Beer-Lambert Law
2. Growth model: Steele's (1977) with both the limitation and inhibition constant
3. Instant and well mixed culture (from modeling standpoint)
4. C, N, P, pH, temperature, vital minerals are not accounted in.
'''

import math


EPSL = 0.15    # m3/m-g
#IN_LI0 = 600    # uE/m2/s
MAX_DEPTH = 5E-02   # 5cm
INI_BIOMASS_CONC = 500  # g/m3 ~ mg/l
K_LI = 30   # light intensity at a half umax
K_IH = 300  # light intensity that inhibits a half umax
U_MAX = 2.5  # maximum growth rate
MNT_RATE = 0.10  # maintanance rate
STEPS_DAY = 1440    # iterations
SOLAR_FILE = '/home/live/Desktop/helloPy/aPBR/z9398232.txt'  # absolute filepath to light profile

def getLightProfile(steps, filename=SOLAR_FILE):
    light_profile = list()
    with open(filename , 'r') as f:
        f.readline()  #  pass the header
        for i in range(0, steps):
            _, _, pwr = f.readline().split(',')
            light_profile.append(float(pwr))
    light_profile = [i*4.6*0.42 for i in light_profile]
    return light_profile


def AILI(li, biomass_conc, esp=EPSL):
    '''return average light intensity based Beer-Lambert Law'''
    _tmp = esp*MAX_DEPTH*biomass_conc
    try:
        math.exp(_tmp)
    except OverflowError:
        print("Exponential func. is too big for this computer")
        raise SystemExit
    return li*(1-1/math.exp(_tmp))/_tmp

def growthRate(aili, *args, **kwargs):
    '''calculate specific growth rate from only AILI using Steele\'s model'''
    u_growth= U_MAX*aili/(aili+K_LI + aili**2/K_IH)
    return u_growth - MNT_RATE


def biomassConc(biomassC, growthrate, delta_t):
    '''return biomass concentration'''
    delta_t /=(60*24)  # convert increment of 1 minutes (light), to growth rate (1/day)
    return biomassC*math.exp(growthrate*delta_t)

def x2List(key, input):
    '''extract data from the bundle to a list'''
    holder = []
    for elem in input:
        holder.append(elem[key])
    return holder


def drawing(result):
    import matplotlib.pyplot as plt
    
    x_axis = [i/60 for i in range(0, steps)]  # convert minutes to hour
    aili_list = x2List('aili', result)
    li0_list = x2List('li0', result)
    ur_list = x2List('ur', result)
    cbm_list = x2List('cbm', result)
    
    bpr_list = list()
    for i in range(1, len(cbm_list)):
        dt_cbm = cbm_list[i] - cbm_list[i-1]
        bpr_list.append(dt_cbm)

    bpr = 0
    for i in range(len(bpr_list)):
        bpr += bpr_list[i]

    ax1 = plt.subplot(4, 1, 1)
    ax1.plot(x_axis, li0_list, 'r')
    ax1.set_title("Simulation: microalgae with PHX solar profile")
    ax1.set_xlabel('time, hr')
    ax1.set_ylabel('LI, uE/m2-s',color='r')
    ax1.tick_params('y',colors='r')
    ax1.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax1.text(0.01, 1500, "epsilon: {}".format(EPSL), {'color':'b', 'fontsize':10})
    ax1.text(0.01, 1000, "light: 2013.7.1", {'color':'b', 'fontsize':10})
    
    ax2 = ax1.twinx()
    ax2.plot(x_axis, aili_list, 'b')
    ax2.set_ylabel('AILI', color='b')
    ax2.set_ylim(0, max(aili_list)*1.2)

    ax3 = plt.subplot(4, 1, 2)
    ax3.plot(x_axis, ur_list, color='green')
    Y3_MAX = max(ur_list)
    ax3.set_ylabel('net growth rate')
    ax3.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax3.text(0.01, 0.7*Y3_MAX, "K_LI: {}".format(K_LI), {'color':'b', 'fontsize':10})
    ax3.text(0.01, 0.5*Y3_MAX, "K_IH: {}".format(K_IH), {'color':'b', 'fontsize':10})
    ax3.text(0.01, 0.3*Y3_MAX, "u_max: {}".format(U_MAX), {'color':'b', 'fontsize':10})
    ax3.text(0.01, 0.1*Y3_MAX, "u_mnt: {}".format(MNT_RATE), {'color':'b', 'fontsize':10})
    
    ax4 = plt.subplot(4, 1, 3)
    ax4.plot(x_axis, cbm_list, color='black')
    ax4.set_ylabel('biomass Conc. (mg/l)')
    ax4.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax4.text(0.01, INI_BIOMASS_CONC*(1.1), "ini_biomass: {}".format(INI_BIOMASS_CONC), {'color':'b', 'fontsize':10})

    ax5 = plt.subplot(4, 1, 4)
    ax5.plot(x_axis[:-1], bpr_list, color ='black')
    ax5.set_ylabel('BPR. (mg/l/min)')
    ax5.set_xlabel('time, hr')
    ax5.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax5.text(0.01, 0.01, "BP(g/l): %.02f" % (bpr/1000), {'color':'b', 'fontsize':10})

    plt.show()


def simulating(light_profile):
    '''simulating with n=steps, default = 1000'''
    result = list()
    # delta_t = DURATION/steps  # 1 minutes
    delta_t = 1  # light profile has one minute measurement
    for i in range(steps):
        snapshot = dict()
        if i == 0:
            _aili = AILI(0, INI_BIOMASS_CONC)
            _u_rate = growthRate(_aili)
            _biomass = biomassConc(INI_BIOMASS_CONC, _u_rate, delta_t)

            snapshot['li0'] = 0
            snapshot['aili'] = _aili
            snapshot['ur'] = _u_rate
            snapshot['cbm'] = _biomass
            result.append(snapshot)

        else:
            li0 = light_profile[i]
            _aili = AILI(li0,_biomass)
            _u_rate = growthRate(_aili)
            _biomass = biomassConc(_biomass, _u_rate, delta_t)
            snapshot['li0'] = li0
            snapshot['aili'] = _aili
            snapshot['ur'] = _u_rate
            snapshot['cbm'] = _biomass
            result.append(snapshot)
    return result

if __name__ == '__main__':
    import time
    t1 = time.time()
    DURATION = 2  #  day
    steps = int(DURATION*STEPS_DAY)
    light_profile = getLightProfile(steps)
    run = simulating(light_profile)
    duration = time.time() - t1
    print("Performing %d steps in %.02f seconds" % (steps, duration))
    drawing(run)
    print('Done')

