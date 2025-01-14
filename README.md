# Projekt: Realtidssystem för väderövervakning

### Designa ett system som simulerar en väderövervakningsstation med hjälp av C++-trådar och periodic tasks. Systemet ska samla in, bearbeta och visa data från flera "sensorer" (temperatur, luftfuktighet och vindhastighet) i realtid.

1. Simulering av sensorer:
o Använd trådar för att simulera tre sensorer: temperatur, luftfuktighet och
vindhastighet.
o Varje sensor genererar slumpmässiga data för Svenska förhållanden inom ett
fördefinierat intervall med jämna tidsintervall (t.ex. var 500:e millisekund).

2. Datainsamling:
o Använd ett delat minnesområde (skyddat av mutex) för att lagra de senaste
data från alla sensorer.
o Implementera flaggor för att signalera när ny data är tillgänglig.

3. Databearbetning:
o Implementera en separat tråd för att beräkna statistik (t.ex. medelvärde, min,
max) för varje sensor var femte sekund baserat på tidigare mätningar.

4. Datavisning:
o Skapa en periodisk uppgift som visar i konsolen de senaste data varannan
sekund och statistiken var tionde sekund.

Program written by Erik Dahl as part of the Chas Academy Software Developer education.

Dependencies:
/nhlomanm/json.hpp single include header file 
Nlohmann json parser - details on how to use this:
https://github.com/nlohmann/json

Compile with main.cpp, DataGenerator.cpp, SensorData.cpp, threads.cpp, globals.cpp, SaveJson.cpp

Sensors generate an initial values, and then fluctuates within a range, never exceeding min/max.
The values could be plausible if you squint your eyes.

Statistics are calculated on the m_new_readings vector, and the values are then moved to another vector m_readings

The program will run until you press q (+ Enter)
It will then save the data in a new json file with name SensorData-<Date>(-<index>).json

