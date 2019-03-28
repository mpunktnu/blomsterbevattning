# blomsterbevattning
Z-wave baserad blomsterbevattning med en Z-uno. Läs mer här: XXYYZZ

## Tillgängliga parametrar
64 - Tid i minuter mellan varje fuktmätning\
65 - Tid i sekunder mellan varje fuktmätning vid bevattning. OBS - sätt aldrig till lägre än 30 sekunder då det är ett hårt krav i Z-Wave + standarden\
66 - Automationstyp:\
0 = All automatik avstängd\
1 = Bevattning sker om fuktighet är under värdet inställt i parameter 80 - 83. Tid för bevattning är enligt parameter 69\
2 = Bevattning sker om fuktighetsvärdet är under parameter 70-73 och vattnar tills värdet i parameter 80-83 är uppnått.\
67 - Det lägsta motståndet som kan uppmätas (i.e värdet i blöt jord)\
68 - Det högsta motståndet som kan uppmätas (i.e värdet i torr jord)\
69 - Tiden i minuter som bevattning skall ske i automationsläge 1\
70 - Fuktighetsgraden i % för start av bevattning med pump 1\
71 - Fuktighetsgraden i % för start av bevattning med pump 2\
72 - Fuktighetsgraden i % för start av bevattning med pump 3\
73 - Fuktighetsgraden i % för start av bevattning med pump 4\
80 - Fuktighetsgraden i % för stopp av bevattning med pump 1\
81 - Fuktighetsgraden i % för stopp av bevattning med pump 2\
82 - Fuktighetsgraden i % för stopp av bevattning med pump 3\
83 - Fuktighetsgraden i % för stopp av bevattning med pump 4\
90 - Om automationen skall gå igång automatiskt vid uppstart\
91 - Antal minuter som bevattningen max får köras. Prioriteras alltid över parameter 69 och parameter 80-83
