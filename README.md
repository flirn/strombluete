# STROMBLÜTE

STROMBLÜTE ist eine multimediale Installation, die mit Roboterblumen Aufmerksamkeit für aussterbende Pflanzenarten erweckt. Hier wird der Aufbau kurz erklärt.

## fertige Blume
![stromblüte](img/strombluete.jpg)

## STL

![3D-Modell 1](img/3d1.png)
![3D-Modell 2](img/3d2.png)

## Bestandteile
Jede Stromblüte besteht aus 8 verschiedenen (18 einzelnen) 3d-gedruckten Bestandteilen. 
Jedes Blütenblatt besteht aus einem Zahnradteil und dem tatsächlichen Blütenblatt. Der Blütenkopf besteht aus 3 Teilen – einer Basis, einem durchsichtigen Kopfteil und einem runden Zahnradstempel. Die Basis baut sich aus einem Hohlkörper, einem Deckel und einem Stützzylinder auf. Sie beinhaltet fast alle elektronischen Elemente. Zusätzlich werden zwei ineinanderpassende Messingrohre (4 mm und 2 mm Durchmesser) benötigt.

![Aufbau Bestandteile](img/AufbauBestandteile.png)

## Elektronik

In jeder einzelnen Blume sind 5 Elektronikelemente verbaut. Die Steuerung übernimmt ein NodeMCU mit dem Modul ESP-32, der über einen Lithium-Polymer-Akku betrieben wird. Der Akku kann mithilfe einer Ladeplatine über USB-C geladen werden. Der Mikrocontroller steuert einen LED-Ring mit 8 LEDs und einen kleinen Servomotor, welcher für das Öffnen und Schließen der Blütenblätter zuständig ist.

![Aufbau Elektronik](img/AufbauElektronik.png)
![Elektronik der fertigen Blume](img/elektronik.jpg)

## Netzwerk

Um jede Blume einzeln ansteuern zu können, muss jede Blume auf ein Signal des Computers reagieren können. Die Komposition der öffnenden und schließenden Blumen läuft dabei über den Musikstandard MIDI. Diese MIDI-Note-Signale werden über die Digital Audio Workstation (DAW) Ableton Live erzeugt und an einen Virtuellen Midi Port gesendet. Dieser gibt das Signal zurück an die DAW, um einen Ton zu erzeugen und wird gleichzeitig über ein Python Programm ausgelesen. Jede einzelne Blume ist per WLAN mit dem Python Websocket Server verbunden und reagiert auf das ihr zugeordnete MIDI-Signal. 

![Aufbau Netzwerk](img/AufbauNetzwerk.png)
