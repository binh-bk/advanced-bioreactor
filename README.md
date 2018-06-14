
# Advanced-bioreactor
the components and codes were built for an algae cultivation vessel using inorganic carbon (CO2), and light energy (from LED or the Sun), but the principle works for other systems, e.g. indoor plantation, other microbial culvation systems.

## I. The following components are hosted here:

- high powered LED control that mimicking a pre-defined solar pattern for one day. More details on instructables.com: https://www.instructables.com/id/Control-High-powered-LED-Panel-by-Arduino-Real-Tim/
- data logger for pH, DO, and temperature, timestamp. More details here: https://www.instructables.com/id/How-to-Make-a-Data-Logger-for-the-Temperature-PH-a/
- a turbidity monitoring and control using infra-red sensor, applied to measure and control algal biomass density. Link to a journal publication: https://www.sciencedirect.com/science/article/pii/S2211926417307683
the 1st code dated _0726, and the cleanup version dated _2018.

## II. Schematic of each module:
### 1. LED Control and Lux Measurement

<p align="center">
  <img src="https://github.com/binh-bk/advanced-bioreactor/blob/master/LED_Control_0513/LED%20control.jpg"/>
</p>

### 2.  Data Logger
<p align="center">
  <img src="https://github.com/binh-bk/advanced-bioreactor/blob/master/Logging_ph_DO_temp_sentdata_I2C_0426/data%20logger_pH_temp_DO.jpg"/>
</p>


### 3.  Turbidity monitoring and control
<p align="center">
  <img src="https://github.com/binh-bk/advanced-bioreactor/blob/master/Turbidity_logdata_YunShield_0726/turbidostat.png"/>
</p>

----
=======
Arduino code for pH, temperature monitoring, data logger, turbiostat, control LED for mimicking a solar pattern

