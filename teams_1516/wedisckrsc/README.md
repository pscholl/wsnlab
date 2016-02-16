### Setup:
Das Netzwerk besteht aus intel edisons, auf denen jeweils Contiki läuft.
Statt dem standartmäßig installierten Yocto wurde Ubininux verwendet, um leichter Paktekte installieren zu können. Neben dem aktuellsten Ubilinux wurde außerdem die aktuellste Version von mraa verwendet (Stand Januar 2016).
Das Contiki basiert auf einem UDP-Beispiel. Es gibt einen Server und mehrere (3 wurden getestet) Clients mit einem Platipus Board.
Der Server öffnet einen WLAN-Accesspoint, die Clients wählen sich ein. Die Kommunikation erfolgt über IPv6, der jeweilige Addresspräfix wird als Argument beim Aufruf übergeben.
Um die Anwendung zu starten muss auf jedem client das configure_routes.py script ausgeführt werden.
Auf dem Master muss das Script master-routes.sh ausgeführt werden.

#### Code
Da an Contiki viele Änderungen vorgenommen werden mussten, wird es mit eingereicht.
Der eigentliche Programmcode liegt im Contiki Ordner unter /apps/wsn-client bzw /apps/wsn-server.
Die Hauptroutinen für die Applikation sowie die Prozesse befinden sich in wsn.c, wiederverwendbare Teile der Software sind in c- und h-Files ausgelagert.

### Teammitglieder
Marc S., Niklas D., Thomas S., Simon K., Daniel W.
