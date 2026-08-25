# NOVAWATCH — Câblage précis V1

> Ce document fixe l'architecture électrique de la première version. Les valeurs de résistances des LED doivent être validées avec la référence exacte des LED avant assemblage définitif.

## 1. Composants de cette architecture

### Affichage

- Arduino Nano 5 V
- 1 × MAX7219, boîtier DIP-24 de préférence pour le prototype
- 114 × LED rouge 3 mm : 112 pour les segments + 2 pour `:`
- 112 × résistances individuelles pour les LED de segments
- 2 × résistances pour les LED du colon
- 1 × résistance RSET pour le MAX7219
- 1 × condensateur 10 µF près du MAX7219
- 1 × condensateur 100 nF près du MAX7219

### Contour

- 68 × LED rouge 3 mm
- 68 × LED verte 3 mm
- 68 × LED bleue 3 mm
- 7 × 74HC595
- 7 × ULN2803A
- 204 résistances individuelles pour les LED du contour, une par LED

### Commande

- 1 × DS3231 RTC
- 4 × boutons poussoirs
- 1 × buzzer passif
- alimentation 5 V régulée capable de fournir plusieurs ampères

---

## 2. Hypothèse indispensable pour l'afficheur

Le MAX7219 pilote directement un afficheur **à cathode commune**. Notre afficheur artisanal doit donc être câblé comme un ensemble de quatre chiffres à cathode commune.

Pour chaque chiffre :

```text
Segment A = 4 LED rouges en parallèle fonctionnel,
mais CHAQUE LED possède sa propre résistance.

+5V / SEG-A du MAX7219
       |
       +-- R_A1 -- LED_A1 --+
       +-- R_A2 -- LED_A2 --+
       +-- R_A3 -- LED_A3 --+---- DIGx
       +-- R_A4 -- LED_A4 --+
```

Même principe pour B, C, D, E, F et G.

**Ne jamais mettre plusieurs LED en parallèle sans résistance individuelle.**

Les quatre cathodes communes du chiffre sont reliées à la sortie DIG correspondante du MAX7219.

---

## 3. Correspondance des segments

Chaque chiffre utilise la nomenclature standard :

```text
       A
      ---
   F |   | B
      - G -
   E |   | C
      ---
       D
```

Le MAX7219 utilise :

```text
SEG A -> segment A
SEG B -> segment B
SEG C -> segment C
SEG D -> segment D
SEG E -> segment E
SEG F -> segment F
SEG G -> segment G
SEG DP -> NON UTILISÉ
```

Le DP reste non connecté.

---

## 4. Câblage MAX7219 vers les 4 chiffres

Nous utilisons uniquement DIG0 à DIG3.

```text
MAX7219             Afficheur
-------------------------------------------
SEG A               tous les segments A
SEG B               tous les segments B
SEG C               tous les segments C
SEG D               tous les segments D
SEG E               tous les segments E
SEG F               tous les segments F
SEG G               tous les segments G
DIG0                cathode commune chiffre 1
DIG1                cathode commune chiffre 2
DIG2                cathode commune chiffre 3
DIG3                cathode commune chiffre 4
DIG4..DIG7          non utilisés
SEG DP              non utilisé
```

Ordre logique de l'affichage :

```text
DIG0 = dizaine des heures
DIG1 = unité des heures
DIG2 = dizaine des minutes
DIG3 = unité des minutes
```

Le logiciel pourra inverser l'ordre si la disposition physique l'impose.

---

## 5. Brochage physique du MAX7219 DIP-24

D'après la fiche technique Analog Devices/Maxim :

```text
Pin 1  = DIN
Pin 2  = DIG0
Pin 3  = DIG4
Pin 4  = GND
Pin 5  = DIG6
Pin 6  = DIG2
Pin 7  = DIG3
Pin 8  = DIG7
Pin 9  = GND
Pin 10 = DIG5
Pin 11 = DIG1
Pin 12 = LOAD / CS
Pin 13 = CLK
Pin 14 = SEG A
Pin 15 = SEG F
Pin 16 = SEG B
Pin 17 = SEG G
Pin 18 = ISET
Pin 19 = V+
Pin 20 = SEG C
Pin 21 = SEG E
Pin 22 = SEG DP
Pin 23 = SEG D
Pin 24 = DOUT
```

Câblage alimentation :

```text
MAX7219 pin 19 (V+) -> +5 V
MAX7219 pin 4       -> GND
MAX7219 pin 9       -> GND
```

Ajouter au plus près du MAX7219 :

```text
+5V ----||---- GND
        100 nF

+5V ----||---- GND
         10 µF
```

---

## 6. RSET du MAX7219

RSET relie la broche ISET à +5 V.

Pour le premier prototype, utiliser la valeur recommandée de la fiche technique pour une configuration standard et régler ensuite la luminosité dans le logiciel.

**Important : ne pas choisir définitivement RSET uniquement à partir d'une estimation de courant des LED.** Les 4 LED d'un segment sont alimentées par des branches parallèles et le courant total d'un segment doit rester dans les limites du MAX7219.

Le MAX7219 est spécifié pour des afficheurs à cathode commune et peut piloter jusqu'à 8 chiffres multiplexés. Sa fiche technique indique un courant de segment maximal recommandé de 40 mA et recommande un condensateur de 10 µF ainsi qu'un 100 nF près du circuit.

---

## 7. Les deux LED du colon `:`

Le colon n'est pas connecté au SEG DP du MAX7219.

Nous avons deux LED rouges indépendantes :

```text
Arduino D7 ---- Rcolon1 ----|>|---- GND
Arduino D7 ---- Rcolon2 ----|>|---- GND
```

Chaque LED possède sa propre résistance.

Les deux LED s'allument ensemble pour former :

```text
HH : MM
   ^
   |
 deux LED rouges
```

Le logiciel pourra les faire clignoter indépendamment si nécessaire.

---

# 8. Brochage Arduino Nano

```text
Arduino Nano        Fonction NOVAWATCH
-----------------------------------------------
D2                  Bouton ON/OFF
D3                  Bouton MODE / RESET / validation
D4                  Bouton +
D5                  Bouton -
D6                  Buzzer passif
D7                  Colon `:`
D8                  DATA des 74HC595
D9                  LATCH des 74HC595
D10                 CS / LOAD du MAX7219
D11                 DIN du MAX7219
D12                 Libre pour extension
D13                 CLOCK partagé MAX7219 + 74HC595
A4                  SDA du DS3231
A5                  SCL du DS3231
5V                  Rail +5 V
GND                 Masse commune
```

Le D13 est partagé comme horloge série. Les deux circuits ont des lignes DATA différentes et des lignes LATCH/CS différentes, donc ils peuvent utiliser la même horloge.

---

# 9. MAX7219 vers Arduino Nano

```text
Arduino Nano       MAX7219
--------------------------------
D11                DIN pin 1
D10                LOAD/CS pin 12
D13                CLK pin 13
5V                 V+ pin 19
GND                GND pins 4 et 9
```

La broche DOUT du MAX7219 n'est pas utilisée dans la V1.

---

# 10. Contour : pas de LED RGB intégrées

NOVAWATCH utilise trois familles de LED indépendantes :

```text
ROUGE : 68 LED
VERT  : 68 LED
BLEU  : 68 LED
```

Chaque couleur est physiquement séparée.

Comme 68 / 4 = 17, nous créons :

```text
17 groupes ROUGES de 4 LED
17 groupes VERTS  de 4 LED
17 groupes BLEUS  de 4 LED

Total = 51 groupes
```

Un groupe correspond à quatre LED successives du contour.

---

# 11. Câblage d'un groupe de 4 LED du contour

Chaque LED possède sa propre résistance.

Exemple pour un groupe rouge :

```text
+5V
 |
 +-- R1 -- LED rouge 1 --+
 +-- R2 -- LED rouge 2 --+
 +-- R3 -- LED rouge 3 --+---- ULN2803A OUT1
 +-- R4 -- LED rouge 4 --+
```

Le ULN2803A est utilisé comme interrupteur côté masse :

```text
ULN2803A OUT1 -> cathodes du groupe
ULN2803A GND  -> GND
```

Le même principe est utilisé pour les groupes verts et bleus.

**Une résistance par LED est obligatoire.**

---

# 12. Pourquoi 7 × 74HC595 et 7 × ULN2803A ?

51 groupes doivent être commandés :

```text
17 rouge + 17 vert + 17 bleu = 51 sorties
```

Un 74HC595 possède 8 sorties :

```text
51 sorties nécessaires
51 / 8 = 6,375
```

Donc il faut **7 × 74HC595**, soit 56 sorties disponibles.

Chaque sortie du 74HC595 commande une entrée du ULN2803A. Les LED ne sont pas alimentées directement par le 74HC595.

Le ULN2803A fournit les interrupteurs de puissance côté masse. Il contient 8 canaux Darlington et est prévu pour les applications de pilotage de LED et autres charges.

---

# 13. Chaînage des 74HC595

```text
Arduino D8 (DATA)
       |
       v
+------------+
| 74HC595 #1 |
+-----+------+
      | Q7S
      v
+------------+
| 74HC595 #2 |
+-----+------+
      | Q7S
      v
+------------+
| 74HC595 #3 |
+-----+------+
      |
     ...
      |
      v
+------------+
| 74HC595 #7 |
+------------+
```

Les lignes communes :

```text
Arduino D13 -> SHCP / CLOCK de tous les 74HC595
Arduino D9  -> STCP / LATCH de tous les 74HC595
+5V         -> VCC de tous les 74HC595
GND         -> GND de tous les 74HC595
```

Pour chaque 74HC595 :

```text
MR / SRCLR -> +5V
OE         -> GND
```

Pour un prototype plus robuste, ajouter un condensateur 100 nF près de chaque 74HC595.

---

# 14. Attribution des 56 sorties

Nous réservons 51 sorties aux groupes de contour et 5 sorties restent libres.

```text
74HC595 #1
Q0 -> Rouge groupe 01
Q1 -> Rouge groupe 02
Q2 -> Rouge groupe 03
Q3 -> Rouge groupe 04
Q4 -> Rouge groupe 05
Q5 -> Rouge groupe 06
Q6 -> Rouge groupe 07
Q7 -> Rouge groupe 08

74HC595 #2
Q0 -> Rouge groupe 09
Q1 -> Rouge groupe 10
Q2 -> Rouge groupe 11
Q3 -> Rouge groupe 12
Q4 -> Rouge groupe 13
Q5 -> Rouge groupe 14
Q6 -> Rouge groupe 15
Q7 -> Rouge groupe 16

74HC595 #3
Q0 -> Rouge groupe 17
Q1 -> Vert groupe 01
Q2 -> Vert groupe 02
Q3 -> Vert groupe 03
Q4 -> Vert groupe 04
Q5 -> Vert groupe 05
Q6 -> Vert groupe 06
Q7 -> Vert groupe 07

74HC595 #4
Q0 -> Vert groupe 08
Q1 -> Vert groupe 09
Q2 -> Vert groupe 10
Q3 -> Vert groupe 11
Q4 -> Vert groupe 12
Q5 -> Vert groupe 13
Q6 -> Vert groupe 14
Q7 -> Vert groupe 15

74HC595 #5
Q0 -> Vert groupe 16
Q1 -> Vert groupe 17
Q2 -> Bleu groupe 01
Q3 -> Bleu groupe 02
Q4 -> Bleu groupe 03
Q5 -> Bleu groupe 04
Q6 -> Bleu groupe 05
Q7 -> Bleu groupe 06

74HC595 #6
Q0 -> Bleu groupe 07
Q1 -> Bleu groupe 08
Q2 -> Bleu groupe 09
Q3 -> Bleu groupe 10
Q4 -> Bleu groupe 11
Q5 -> Bleu groupe 12
Q6 -> Bleu groupe 13
Q7 -> Bleu groupe 14

74HC595 #7
Q0 -> Bleu groupe 15
Q1 -> Bleu groupe 16
Q2 -> Bleu groupe 17
Q3-Q7 -> réservées pour extension
```

---

# 15. Correspondance 74HC595 -> ULN2803A

Chaque sortie Q du 74HC595 va vers une entrée IN du ULN2803A correspondant.

Exemple :

```text
74HC595 #1 Q0 -> ULN2803A #1 IN1
ULN2803A #1 OUT1 -> cathodes des 4 LED rouges du groupe 01
```

Même principe pour les 51 groupes.

Les 7 ULN2803A ont leur GND relié à la masse commune.

Les sorties non utilisées du dernier ULN2803A restent non connectées.

---

# 16. Alimentation

Architecture recommandée pour le prototype :

```text
Prise JACK DC
     |
     v
Alimentation DC externe
     |
     v
Convertisseur abaisseur (buck)
     |
     +---- +5V principal -------------------+
     |                                      |
     |                                      +--> Arduino Nano 5V
     |                                      +--> MAX7219
     |                                      +--> DS3231
     |                                      +--> 74HC595 x7
     |                                      +--> LEDs contour
     |                                      +--> colon
     |
     +---- GND commun ----------------------+
```

Pour le prototype, viser une alimentation **5 V régulée d'au moins 3 A**, avec marge supplémentaire recommandée si plusieurs groupes du contour doivent être allumés simultanément.

Ne pas alimenter les 200+ LED de contour depuis le régulateur 5 V du Nano.

Toutes les masses doivent être communes.

---

# 17. Découplage

Minimum :

```text
MAX7219 : 100 nF + 10 µF
Chaque 74HC595 : 100 nF
DS3231 : 100 nF si le module n'en possède pas déjà
```

Les condensateurs doivent être placés physiquement près des circuits intégrés.

---

# 18. Résistances des LED — première estimation

Les LED 3 mm n'ont pas toutes la même tension directe. Les valeurs finales seront calculées à partir de la référence exacte des LED.

Pour le contour, on commence avec une résistance individuelle plutôt élevée, par exemple **680 Ω**, afin de privilégier la sécurité lors du premier test.

Exemple pour une LED rouge approximativement Vf = 2,0 V à 5 V :

```text
R = (5 V - 2 V) / 0,0044 A
R ≈ 680 Ω
```

Le courant réel dépendra de la LED et de la tension directe.

Pour les LED vertes et bleues, Vf est différente : ne pas supposer qu'elles ont la même Vf que les rouges.

---

# 19. Résumé du câblage

```text
                         +5V
                          |
        +-----------------+------------------+
        |                 |                  |
        v                 v                  v
     MAX7219            DS3231          74HC595 x7
        |                                      |
        |                                      v
        |                                  ULN2803A x7
        |                                      |
        v                                      v
   4 chiffres                            51 groupes
   112 LED                                de 4 LED
      +                                       |
  2 colon                                R / V / B
      |
      v
   HH : MM
```

---

# 20. Tests obligatoires avant assemblage final

### Test 1

Construire seulement **un chiffre** avec 7 segments et 4 LED par segment.

### Test 2

Vérifier chaque LED et chaque résistance individuellement.

### Test 3

Connecter ce chiffre au MAX7219.

### Test 4

Afficher :

```text
0
1
2
3
4
5
6
7
8
9
```

### Test 5

Construire les 4 chiffres et tester :

```text
00:00
12:34
88:88
23:59
```

### Test 6

Construire un seul groupe de 4 LED rouges du contour avec un 74HC595 + ULN2803A.

### Test 7

Tester un groupe vert puis bleu.

### Test 8

Valider les 51 groupes avant de fabriquer le contour complet.

---

## Sources techniques

- MAX7219 : Analog Devices / Maxim — pilote jusqu'à 8 chiffres, multiplexage intégré et affichage à cathode commune.
- 74HC595 : Nexperia / Texas Instruments — registre à décalage 8 bits avec sorties parallèles, adapté à l'extension d'E/S.
- ULN2803A : Texas Instruments / STMicroelectronics — réseau de 8 transistors Darlington utilisé ici comme drivers côté masse.
