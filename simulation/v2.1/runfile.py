#! /usr/bin/python3
'''
Binh Nguyen, July 1, 2018
A simulation using Python3 to model algal biomass production:
input: using solar profile from Phoenix, AZ, USA
constant: marked as UPPER_CASE below
key assumptions:
1. Light intensity: calculated as the volume average based the Beer-Lambert Law
2. Growth model: Aiba's (1982) with both the limitation and inhibition constant
3. Instant and well mixed culture (from modeling standpoint)
4. C, N, P, pH, temperature, vital minerals are not accounted in.
'''

import math, time, os, sys, csv
import logtime

STEPS_DAY = 1440    # iterations a day

default = {
    'epsl': 0.15,
    'max_depth': 1E-02,
    'ini_biomass': 1000,
    'k_li': 30,
    'k_ih': 300,
    'u_max': 2.5,
    'u_mnt': 0.1,
    'solar_profile': 'winter',
    'graph_output': '/home/live/Desktop/v2.1/',
    'graph_save': True,
    'mode': 'turbido',  # turbido or batch
    'duration': 2,
}

def get_input(inputfile):
    setups = list()
    with open(inputfile, 'r') as f:
        reader = csv.DictReader(f)
        for row in reader:
            setups.append(row)
    return setups   

def assign_input(input_):
    global EPSL, MAX_DEPTH, INI_BIOMASS_CONC, SET_BIOMASS_C, K_IH, K_LI, U_MAX, MNT_RATE, SOLAR_FILE, IMG_FILE, SPECIFIC_AREA, DURATION, MODE, GRAPHFLAG, SEASON
    if input_['solar_profile'] == 'winter':
        SOLAR_FILE = 'z3286089.txt' 
        SEASON = 'w' 
    elif input_['solar_profile'] == 'summer':
        SOLAR_FILE = 'z9398232.txt'
        SEASON = 's'  
    else:
        SOLAR_FILE = None
    U_MAX = float(input_['u_max'])
    MNT_RATE = float(input_['u_mnt'])
    K_IH = int(input_['k_ih'])
    K_LI = int(input_['k_li'])
    EPSL = float(input_['epsl'])
    INI_BIOMASS_CONC = int(input_['ini_biomass'])
    SET_BIOMASS_C = round(1.1*INI_BIOMASS_CONC)
    MAX_DEPTH = float(input_['max_depth'])
    SPECIFIC_AREA = 1/MAX_DEPTH
    DURATION = int(input_['duration'])
    GRAPHFLAG = bool(input_['graph_save'])
    IMG_FILE = input_['graph_output']
    MODE = input_['mode']
    print("Inputs X0 {}".format(INI_BIOMASS_CONC))
    return None


def datestamp():
    date_ = time.strftime('%Y%m%d', time.localtime())
    specs = '.'.join([SEASON, str(INI_BIOMASS_CONC), str(MAX_DEPTH), str(EPSL),'png'])
    f_name = '-'.join([date_, specs])
    return IMG_FILE + f_name

def getdate(filename):
    '''return the date of light profile'''
    with open(filename, 'r') as f:
        f.readline()
        date_, _, _ = f.readline().split(',')
    return date_

def getLightProfile(steps, filename):
    '''return the light intensity in PAR '''
    light_profile = list()
    with open(filename , 'r') as f:
        f.readline()  #  pass the header
        for _ in range(0, steps):
            _, _, pwr = f.readline().split(',')
            light_profile.append(float(pwr))
    light_profile = [i*4.6*0.42 for i in light_profile]
    return light_profile


def AILI(li, biomass_conc, esp):
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
    return U_MAX*aili/(aili+K_LI + aili**2/K_IH)

def netGrowthRate(grossGrowthrate, growthmode, biomass):
    if growthmode == 'batch':
        dilution_rate = 0
        net_rate = grossGrowthrate - MNT_RATE
    elif growthmode == 'turbido':
        if biomass < (SET_BIOMASS_C-1):
            dilution_rate = 0
            net_rate = grossGrowthrate - MNT_RATE
        else:
            if grossGrowthrate == 0:
                dilution_rate = 0
                net_rate = - MNT_RATE
            else:
                dilution_rate = grossGrowthrate - MNT_RATE
                net_rate = 0
            # net to put some safeguard here.
    return net_rate, dilution_rate


def biomassConc(biomassC, net_rate, delta_t):
    '''return biomass concentration, growthrate = net growth rate'''
    delta_t /=(60*24)  # convert increment of 1 minutes (light), to growth rate (1/day)
    return biomassC*math.exp(net_rate*delta_t)

def x2List(key, collection):
    '''extract data from the bundle to a list'''
    holder = []
    for elem in collection:
        holder.append(elem[key])
    return holder

def biomass_productivity(result):
    '''return a list of biomass productivity (g/m2-day) from simulation '''
    # steps = len(result)
    cbm_list = x2List('cbm', result)
    # ur_list = x2List('ur', result)
    dr_list = x2List('dr', result)
    bpr_list = list()  # mg/L/min
    for i in range(1, len(cbm_list)):
        dt_cbm = cbm_list[i] - cbm_list[i-1] + cbm_list[i]*(dr_list[i]/24/60)
        bpr_list.append(dt_cbm)
    bpr_l_day = [1440*elem/SPECIFIC_AREA for elem in bpr_list]  #g/m2-day per  min internval
    day_ = 0
    global bpr_m2
    bpr_m2 = list()
    for i in range(1, 1440):
        day_ += bpr_l_day[i]
    bpr_m2.append(day_/1440)
    day_ = 0
    for i in range(1441, len(bpr_list)):
        day_ += bpr_l_day[i]
    bpr_m2.append(day_/1440)
    return bpr_m2, bpr_l_day

def drawing(result):
    '''generate graph with 4 subplots and automatically add legends to graph'''
    from matplotlib import rcParams
    rcParams['font.family'] = 'DejaVu Sans'
    rcParams['font.sans-serif'] = ['Tahoma']
    import matplotlib.pyplot as plt

    steps = len(result)
    x_axis = [i/60 for i in range(steps)]  # convert minutes to hours

    aili_list = x2List('aili', result)

    li0_list = x2List('li0', result)
    ur_list = x2List('ur', result)
    dr_list = x2List('dr', result)
    cbm_list = x2List('cbm', result)
    bpr_m2, bpr_l_day = biomass_productivity(result)

    f = plt.figure(figsize=(10,8))
    f.suptitle("Simulation: Microalgae with PHX solar profile\n X0: {},mg/l, depth={},m, K_IH: {}uE/m2-s".format(INI_BIOMASS_CONC, MAX_DEPTH, K_IH), fontsize=12, )

    ax1 = f.add_subplot(4,1,1)
    ax1.plot(x_axis, li0_list,  'r')
    Y1_MAX = max(li0_list)
    ax1.set_title("Incident and Average LI (PAR)")
    #ax1.set_xlabel('time, hr')
    ax1.set_ylabel('LI, uE/m2-s',color='r')
    ax1.tick_params('y',colors='r')
    ax1.grid(color='grey', linestyle='-', linewidth=0.2, )
    ax1.text(-2, Y1_MAX*0.6, "PBR_D,m: {}".format(MAX_DEPTH), {'color':'#0b267a', 'fontsize':10})
    ax1.text(-2, Y1_MAX*0.4, "epsilon: {}".format(EPSL), {'color':'#0b267a', 'fontsize':10})
    ax1.text(-2, Y1_MAX*0.2, "light: {}".format(getdate(SOLAR_FILE)), {'color':'#0b267a', 'fontsize':10})


    ax2 = ax1.twinx()
    ax2.plot(x_axis, aili_list, 'b',)
    ax2.set_ylabel('AILI, uE/m2.s', color='b')
    ax2.set_ylim(0, max(aili_list)*1.2)

    ax3 = f.add_subplot(4, 1, 2)
    ax3.set_title("Growth and Dilution Rates")
    ax3.plot(x_axis, ur_list, color='green')
    ax3.set_ylabel('net growth rate,1/day')
    Y3_MAX = max(ur_list)
    ax3.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax3.text(-2, Y3_MAX*0.7, "K_LI: {}".format(K_LI), {'color':'#0b267a', 'fontsize':10})
    ax3.text(-2, Y3_MAX*0.5, "K_IH: {}".format(K_IH), {'color':'#0b267a', 'fontsize':10})
    ax3.text(-2, Y3_MAX*0.3, "u_max: {}".format(U_MAX), {'color':'#0b267a', 'fontsize':10})
    ax3.text(-2, Y3_MAX*0.1, "u_mnt: {}".format(MNT_RATE), {'color':'#0b267a', 'fontsize':10})
    ax4 = ax3.twinx()
    ax4.plot(x_axis, dr_list, 'r')
    ax4.set_ylabel('dilution_rate,1/day', color='r')

    ax5 = f.add_subplot(4, 1, 3)
    ax5.plot(x_axis, cbm_list, color='black')
    ax5.set_title("Biomass Conc. in PBR")
    ax5.set_ylabel('biomass Conc. (mg/l)')
    ax5.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax5.text(-2, INI_BIOMASS_CONC, "X0: {}".format(INI_BIOMASS_CONC), {'color':'#0b267a', 'fontsize':10})
    if (MODE=='turbido'):
        ax5.text(-2, INI_BIOMASS_CONC*1.03, "Xset: {}".format(SET_BIOMASS_C), {'color':'#0b267a', 'fontsize':10})

    ax6 = f.add_subplot(4, 1, 4)
    ax6.plot(x_axis[:-1], bpr_l_day, color ='black')
    ax6.set_title("Biomass Productivity")
    ax6.set_ylabel('BPR. (g/m2/day)')
    ax6.set_xlabel('time, hr')
    ax6.grid(color='grey', linestyle='-', linewidth=0.5, )
    ax6.text(-2, 0.2, "Daily BPR (g/m2):", {'color':'#0b267a', 'fontsize':10})
    ax6.text(10, 0.2, "day 1: %.1f" % (bpr_m2[0]), {'color':'#0b267a', 'fontsize':10})
    ax6.text(32, 0.2, "day 2: %.1f" % (bpr_m2[1]), {'color':'#0b267a', 'fontsize':10})

    f.tight_layout()
    f.subplots_adjust(top=0.88)
    # plt.show()
    f.savefig(datestamp())
    # plt.clf()
    plt.close()
    print('Save graph file in {}'.format(IMG_FILE))
    return None

@logtime.my_timer
def simulating(light_profile, steps):
    '''simulating with n steps, default 1440 steps (mins) per day * # days'''
    result = list()
    # delta_t = DURATION/steps  # 1 minutes
    delta_t = 1  # light profile has one minute measurement
    for i in range(steps):
        snapshot = dict()
        if i == 0:
            aili_ = AILI(0, INI_BIOMASS_CONC, EPSL)
            gross_u_rate_ = growthRate(aili_)
            net_rate_, dilution_rate_ = netGrowthRate(gross_u_rate_, MODE, INI_BIOMASS_CONC)
            biomass_ = biomassConc(INI_BIOMASS_CONC, net_rate_, delta_t)

            snapshot['li0'] = 0
            snapshot['aili'] = aili_
            snapshot['ur'] = net_rate_
            snapshot['dr'] = dilution_rate_
            snapshot['cbm'] = biomass_
            result.append(snapshot)
        else:
            li0 = light_profile[i]
            aili_ = AILI(li0,biomass_,EPSL)
            gross_u_rate_ = growthRate(aili_)
            net_rate_, dilution_rate_ = netGrowthRate(gross_u_rate_, MODE, biomass_)
            biomass_ = biomassConc(biomass_, net_rate_, delta_t)

            snapshot['li0'] = li0
            snapshot['aili'] = aili_
            snapshot['ur'] = net_rate_
            snapshot['dr'] = dilution_rate_
            snapshot['cbm'] = biomass_
            result.append(snapshot)
    return result

def save_output(outputs):

    fieldnames = ['set#', 'solar_profile', 'max_depth', 'epsl', 'u_max','u_mnt', 'k_li','k_ih','ini_biomass','mode','graph_save','graph_output','steps_day','duration', 'day1','day2']
    with open('output.csv', 'a') as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        for each in outputs:
            writer.writerow({k:v for k,v in each.items()})
    return None


@logtime.my_logger
@logtime.memory_usage_psutil
@logtime.my_timer
def main():
    
    steps = int(DURATION*STEPS_DAY)
    light_profile = getLightProfile(steps,filename=SOLAR_FILE)
    run_output = simulating(light_profile, steps)
    if (GRAPHFLAG):
        drawing(run_output)
    input_['day1'] = bpr_m2[0]
    input_['day2'] = bpr_m2[1]
    outputs.append(input_)
    return None

inputs = list()
if len(sys.argv) >= 2:
    inputfile = sys.argv[1]
    print('Name of input: {}'.format(inputfile))
    inputs = get_input(inputfile)
else:
    print('running with default value {}'.format(default))
    inputs.append(default)


if __name__ == '__main__':
    global outputs
    outputs = list()
    for input_ in inputs:
        if 'set#' in input_.keys():
            print('>>> Running set: {} total: {}'.format(input_['set#'],len(inputs)))
        else:
            print("Running one test.")
        assign_input(input_)
        main()
    save_output(outputs)
    print('Done.')

