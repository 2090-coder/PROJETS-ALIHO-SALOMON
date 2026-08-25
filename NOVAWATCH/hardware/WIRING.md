# NOVAWATCH — Câblage précis V2

> Document de référence du câblage. Toutes les broches des CI sont écrites avec leur nom réel tel qu'il apparaît sur le composant. L'alimentation d'entrée du projet est de **12 V DC**. Les tensions nécessaires sont ensuite distribuées par un convertisseur abaisseur (buck) adapté.

## 1. Architecture d'alimentation

**Entrée principale : 12 V DC par prise JACK.**

Le 12 V ne doit jamais être envoyé directement au MAX7219, au DS3231, au 74HC595 ou aux broches 5 V de l'Arduino Nano.

| Source | Va vers | Tension | Rôle |
|---|---|---:|---|
| Prise JACK | Interrupteur/fusible d'entrée | 12 V DC | Entrée principale |
| Interrupteur/fusible | Buck DC-DC | 12 V DC | Protection + distribution |
| Buck DC-DC | Rail logique | **5 V DC** | Arduino, MAX7219, DS3231, 74HC595 |
| Rail 12 V | Résistances + LEDs contour | **12 V DC** | Alimentation des groupes du contour |
| GND 12 V | GND commun | 0 V | Masse commune du système |

> Le buck doit être réglé à **5,0 V avant de connecter l'Arduino ou les CI**. Pour le prototype, choisir un buck 12 V → 5 V capable de fournir au moins 3 A avec une marge raisonnable. La puissance finale doit être vérifiée après mesure du courant réel du contour.

## 2. Arduino Nano — tableau principal

| Broche Arduino | Va vers | Fonction |
|---|---|---|
| D2 | Bouton 1 | ON/OFF |
| D3 | Bouton 2 | RESET / MODE / validation |
| D4 | Bouton 3 | + |
| D5 | Bouton 4 | - |
| D6 | Buzzer passif | Son |
| D7 | Colon `:` via résistances | Deux LED rouges du colon |
| D8 | **DS (pin 14) du 74HC595 #1** | DATA contour |
| D9 | **STCP (pin 12) des 74HC595 #1 à #7** | LATCH / transfert |
| D10 | **LOAD/CS (pin 12) du MAX7219** | Validation afficheur |
| D11 | **DIN (pin 1) du MAX7219** | DATA afficheur |
| D12 | Libre | Extension future |
| D13 | **CLK (pin 13) du MAX7219 + SHCP (pin 11) des 74HC595 #1 à #7** | CLOCK partagé |
| A4 | SDA du DS3231 | I2C DATA |
| A5 | SCL du DS3231 | I2C CLOCK |
| 5V | Rail +5 V | Alimentation logique |
| GND | Rail GND | Masse commune |

## 3. 74HC595 — nomenclature exacte du CI

> **Attention : le CI utilisé ici est le 74HC595.** On n'utilise pas les noms génériques `SER`, `LATCH` ou `SRCLK` dans les tableaux de câblage. On utilise les noms visibles sur le CI : **DS, SHCP, STCP, MR, OE, Q0-Q7, Q7S, VCC, GND**.

| Broche | Nom exact sur le CI | Va vers | Fonction |
|---:|---|---|---|
| 1 | **Q1** | Entrée ULN2803A correspondante | Sortie logique |
| 2 | **Q2** | Entrée ULN2803A correspondante | Sortie logique |
| 3 | **Q3** | Entrée ULN2803A correspondante | Sortie logique |
| 4 | **Q4** | Entrée ULN2803A correspondante | Sortie logique |
| 5 | **Q5** | Entrée ULN2803A correspondante | Sortie logique |
| 6 | **Q6** | Entrée ULN2803A correspondante | Sortie logique |
| 7 | **Q7** | Entrée ULN2803A correspondante | Sortie logique |
| 8 | **GND** | GND commun | Masse |
| 9 | **Q7S** | **DS (pin 14) du 74HC595 suivant** | Chaînage série |
| 10 | **MR** | +5 V | Reset maintenu inactif |
| 11 | **SHCP** | Arduino D13 | Horloge du registre |
| 12 | **STCP** | Arduino D9 | Transfert registre → sorties |
| 13 | **OE** | GND | Sorties activées |
| 14 | **DS** | Arduino D8 ou Q7S du CI précédent | Entrée série DATA |
| 15 | **Q0** | Entrée ULN2803A correspondante | Sortie logique |
| 16 | **VCC** | +5 V | Alimentation |

### 74HC595 #1 à #7 — commandes communes

| Broche 74HC595 | Nom exact | Connexion |
|---:|---|---|
| 8 | GND | GND commun |
| 10 | MR | +5 V |
| 11 | SHCP | Arduino D13, commun aux 7 CI |
| 12 | STCP | Arduino D9, commun aux 7 CI |
| 13 | OE | GND |
| 16 | VCC | +5 V |

### Chaînage exact des données

| Source | Va vers | Fonction |
|---|---|---|
| Arduino D8 | DS pin 14 du 74HC595 #1 | Premier bit envoyé |
| Q7S pin 9 du #1 | DS pin 14 du #2 | Chaînage |
| Q7S pin 9 du #2 | DS pin 14 du #3 | Chaînage |
| Q7S pin 9 du #3 | DS pin 14 du #4 | Chaînage |
| Q7S pin 9 du #4 | DS pin 14 du #5 | Chaînage |
| Q7S pin 9 du #5 | DS pin 14 du #6 | Chaînage |
| Q7S pin 9 du #6 | DS pin 14 du #7 | Chaînage |
| Q7S pin 9 du #7 | Non connecté | Fin de chaîne |

## 4. MAX7219 DIP-24 — brochage précis

| Broche MAX7219 | Nom | Va vers | Fonction |
|---:|---|---|---|
| 1 | **DIN** | Arduino D11 | Données série |
| 2 | **DIG0** | Cathodes communes du chiffre 1 | Dizaine des heures |
| 3 | **DIG4** | Non connecté | Non utilisé |
| 4 | **GND** | GND commun | Masse |
| 5 | **DIG6** | Non connecté | Non utilisé |
| 6 | **DIG2** | Cathodes communes du chiffre 3 | Dizaine des minutes |
| 7 | **DIG3** | Cathodes communes du chiffre 4 | Unité des minutes |
| 8 | **DIG7** | Non connecté | Non utilisé |
| 9 | **GND** | GND commun | Masse |
| 10 | **DIG5** | Non connecté | Non utilisé |
| 11 | **DIG1** | Cathodes communes du chiffre 2 | Unité des heures |
| 12 | **LOAD/CS** | Arduino D10 | Validation |
| 13 | **CLK** | Arduino D13 | Horloge |
| 14 | **SEG A** | Anodes des LED A via résistances | Segment A |
| 15 | **SEG F** | Anodes des LED F via résistances | Segment F |
| 16 | **SEG B** | Anodes des LED B via résistances | Segment B |
| 17 | **SEG G** | Anodes des LED G via résistances | Segment G |
| 18 | **ISET** | RSET puis +5 V | Réglage du courant |
| 19 | **V+** | +5 V | Alimentation |
| 20 | **SEG C** | Anodes des LED C via résistances | Segment C |
| 21 | **SEG E** | Anodes des LED E via résistances | Segment E |
| 22 | **SEG DP** | Non connecté | Pas de point décimal |
| 23 | **SEG D** | Anodes des LED D via résistances | Segment D |
| 24 | **DOUT** | Non connecté en V1 | Extension éventuelle |

### Découplage MAX7219

| Composant | Va entre | Position |
|---|---|---|
| 100 nF | +5 V ↔ GND | Au plus près du MAX7219 |
| 10 µF | +5 V ↔ GND | Au plus près du MAX7219 |
| RSET | ISET ↔ +5 V | Valeur à déterminer selon le courant cible |

## 5. Afficheur 7 segments artisanal

| Élément | Quantité |
|---|---:|
| LED rouge 3 mm par segment | 4 |
| Segments par chiffre | 7 |
| LED par chiffre | 28 |
| Chiffres | 4 |
| LED des segments | 112 |
| LED rouges du colon `:` | 2 |
| Total affichage | **114** |
| DP | **Non utilisé** |

### Correspondance des segments

| Sortie MAX7219 | Segment physique |
|---|---|
| SEG A | Supérieur |
| SEG B | Supérieur droit |
| SEG C | Inférieur droit |
| SEG D | Inférieur |
| SEG E | Inférieur gauche |
| SEG F | Supérieur gauche |
| SEG G | Central |
| SEG DP | Non utilisé |

### Correspondance des chiffres

| Sortie MAX7219 | Position |
|---|---|
| DIG0 | Dizaine heures |
| DIG1 | Unité heures |
| DIG2 | Dizaine minutes |
| DIG3 | Unité minutes |
| DIG4-DIG7 | Non utilisés |

## 6. Un segment = 4 LED rouges 3 mm

Chaque LED doit avoir **sa propre résistance**.

| Élément | Va vers |
|---|---|
| MAX7219 SEG A | Résistance A1 |
| Résistance A1 | Anode LED A1 |
| MAX7219 SEG A | Résistance A2 |
| Résistance A2 | Anode LED A2 |
| MAX7219 SEG A | Résistance A3 |
| Résistance A3 | Anode LED A3 |
| MAX7219 SEG A | Résistance A4 |
| Résistance A4 | Anode LED A4 |
| Cathodes LED A1-A4 | Cathode commune du chiffre → DIGx |

Même principe pour les segments B, C, D, E, F et G et pour les quatre chiffres.

## 7. Colon `:`

Le colon est indépendant du `SEG DP` du MAX7219.

| Arduino | Va vers | Puis vers |
|---|---|---|
| D7 | Résistance colon haut | Anode LED colon haut |
| D7 | Résistance colon bas | Anode LED colon bas |
| Cathode colon haut | GND | Masse |
| Cathode colon bas | GND | Masse |

> Les deux LED du colon ne doivent pas être reliées directement à D7 sans résistances. La valeur exacte sera fixée selon le courant choisi pour les LED rouges 3 mm.

## 8. Contour — LED séparées par couleur

> **Ce ne sont pas des LED RGB intégrées. Chaque LED possède une seule couleur.**

| Couleur | LED 3 mm | Groupes de 4 |
|---|---:|---:|
| Rouge | 68 | 17 |
| Vert | 68 | 17 |
| Bleu | 68 | 17 |
| **Total** | **204** | **51** |

## 9. Groupe de 4 LED du contour

Le contour est alimenté depuis le **12 V DC**, pas depuis le 5 V de l'Arduino.

Chaque LED possède sa propre résistance.

| Élément | Va vers |
|---|---|
| +12 V | Résistance R1 |
| Résistance R1 | Anode LED 1 |
| +12 V | Résistance R2 |
| Résistance R2 | Anode LED 2 |
| +12 V | Résistance R3 |
| Résistance R3 | Anode LED 3 |
| +12 V | Résistance R4 |
| Résistance R4 | Anode LED 4 |
| Cathodes LED 1-4 | Une sortie OUT du ULN2803A |
| GND ULN2803A | GND commun |

## 10. 74HC595 → ULN2803A → contour

Chaque sortie Q0-Q7 du 74HC595 commande une entrée du ULN2803A. Le ULN2803A commute le côté cathode du groupe de 4 LED.

| 74HC595 | Broche | ULN2803A | Entrée | Groupe |
|---|---:|---|---:|---|
| #1 | Q0 (15) | #1 | IN1 | Rouge 01 |
| #1 | Q1 (1) | #1 | IN2 | Rouge 02 |
| #1 | Q2 (2) | #1 | IN3 | Rouge 03 |
| #1 | Q3 (3) | #1 | IN4 | Rouge 04 |
| #1 | Q4 (4) | #1 | IN5 | Rouge 05 |
| #1 | Q5 (5) | #1 | IN6 | Rouge 06 |
| #1 | Q6 (6) | #1 | IN7 | Rouge 07 |
| #1 | Q7 (7) | #1 | IN8 | Rouge 08 |
| #2 | Q0 (15) | #2 | IN1 | Rouge 09 |
| #2 | Q1 (1) | #2 | IN2 | Rouge 10 |
| #2 | Q2 (2) | #2 | IN3 | Rouge 11 |
| #2 | Q3 (3) | #2 | IN4 | Rouge 12 |
| #2 | Q4 (4) | #2 | IN5 | Rouge 13 |
| #2 | Q5 (5) | #2 | IN6 | Rouge 14 |
| #2 | Q6 (6) | #2 | IN7 | Rouge 15 |
| #2 | Q7 (7) | #2 | IN8 | Rouge 16 |
| #3 | Q0 (15) | #3 | IN1 | Rouge 17 |
| #3 | Q1 (1) | #3 | IN2 | Vert 01 |
| #3 | Q2 (2) | #3 | IN3 | Vert 02 |
| #3 | Q3 (3) | #3 | IN4 | Vert 03 |
| #3 | Q4 (4) | #3 | IN5 | Vert 04 |
| #3 | Q5 (5) | #3 | IN6 | Vert 05 |
| #3 | Q6 (6) | #3 | IN7 | Vert 06 |
| #3 | Q7 (7) | #3 | IN8 | Vert 07 |
| #4 | Q0 (15) | #4 | IN1 | Vert 08 |
| #4 | Q1 (1) | #4 | IN2 | Vert 09 |
| #4 | Q2 (2) | #4 | IN3 | Vert 10 |
| #4 | Q3 (3) | #4 | IN4 | Vert 11 |
| #4 | Q4 (4) | #4 | IN5 | Vert 12 |
| #4 | Q5 (5) | #4 | IN6 | Vert 13 |
| #4 | Q6 (6) | #4 | IN7 | Vert 14 |
| #4 | Q7 (7) | #4 | IN8 | Vert 15 |
| #5 | Q0 (15) | #5 | IN1 | Vert 16 |
| #5 | Q1 (1) | #5 | IN2 | Vert 17 |
| #5 | Q2 (2) | #5 | IN3 | Bleu 01 |
| #5 | Q3 (3) | #5 | IN4 | Bleu 02 |
| #5 | Q4 (4) | #5 | IN5 | Bleu 03 |
| #5 | Q5 (5) | #5 | IN6 | Bleu 04 |
| #5 | Q6 (6) | #5 | IN7 | Bleu 05 |
| #5 | Q7 (7) | #5 | IN8 | Bleu 06 |
| #6 | Q0 (15) | #6 | IN1 | Bleu 07 |
| #6 | Q1 (1) | #6 | IN2 | Bleu 08 |
| #6 | Q2 (2) | #6 | IN3 | Bleu 09 |
| #6 | Q3 (3) | #6 | IN4 | Bleu 10 |
| #6 | Q4 (4) | #6 | IN5 | Bleu 11 |
| #6 | Q5 (5) | #6 | IN6 | Bleu 12 |
| #6 | Q6 (6) | #6 | IN7 | Bleu 13 |
| #6 | Q7 (7) | #6 | IN8 | Bleu 14 |
| #7 | Q0 (15) | #7 | IN1 | Bleu 15 |
| #7 | Q1 (1) | #7 | IN2 | Bleu 16 |
| #7 | Q2 (2) | #7 | IN3 | Bleu 17 |
| #7 | Q3-Q7 | — | — | Réservées |

## 11. Alimentation du contour 12 V

| Élément | Alimentation |
|---|---|
| LED rouges contour | +12 V → résistance → LED → ULN2803A → GND |
| LED vertes contour | +12 V → résistance → LED → ULN2803A → GND |
| LED bleues contour | +12 V → résistance → LED → ULN2803A → GND |
| ULN2803A | Logique 5 V côté entrées + GND commun |
| 74HC595 | +5 V |

Les résistances du contour seront calculées séparément pour rouge, vert et bleu, car leurs tensions directes `Vf` sont différentes.

## 12. DS3231

| Arduino Nano | DS3231 | Fonction |
|---|---|---|
| A4 | SDA | I2C DATA |
| A5 | SCL | I2C CLOCK |
| 5V | VCC | Alimentation |
| GND | GND | Masse |

## 13. Boutons

| Bouton | Broche Arduino | Fonction |
|---|---|---|
| Bouton 1 | D2 | ON/OFF |
| Bouton 2 | D3 | RESET / MODE / validation |
| Bouton 3 | D4 | + |
| Bouton 4 | D5 | - |

Pour le prototype : une borne du bouton vers la broche Arduino et l'autre vers GND, avec `INPUT_PULLUP` dans le code.

## 14. Buzzer passif

| Arduino | Va vers | Fonction |
|---|---|---|
| D6 | Buzzer passif | Son |
| GND | Buzzer | Masse |

Si le buzzer choisi consomme trop de courant, il sera commandé par un transistor. Ce point sera validé avec la référence du buzzer.

## 15. Protection et distribution 12 V → 5 V

| Élément | Connexion | Rôle |
|---|---|---|
| JACK + | Fusible/interrupteur | Entrée +12 V |
| JACK - | GND commun | Retour alimentation |
| +12 V protégé | Entrée `VIN+` du buck | Conversion 12 V → 5 V |
| GND | Entrée `VIN-` du buck | Masse buck |
| Sortie +5 V du buck | Rail +5 V | Logique |
| Sortie GND du buck | Rail GND | Masse logique |
| +12 V | Contour LED | Puissance LEDs |
| +5 V | Arduino Nano | Logique |
| +5 V | MAX7219 | Logique + affichage |
| +5 V | 74HC595 | Logique contour |
| +5 V | DS3231 | RTC |

> **Attention : ne pas brancher le 12 V directement sur la broche 5V de l'Arduino Nano, ni sur le V+ du MAX7219, ni sur VCC des 74HC595/DS3231.**

## 16. Découplage recommandé

| Composant | Va entre | Position |
|---|---|---|
| 100 nF | +5 V ↔ GND | Près de chaque 74HC595 |
| 100 nF | +5 V ↔ GND | Près du MAX7219 |
| 100 nF | +5 V ↔ GND | Près du DS3231 |
| 10 µF | +5 V ↔ GND | Près du MAX7219 / rail logique |
| Condensateur de sortie buck | Selon fiche technique du buck | À proximité du buck |

## 17. Résistances — règle de câblage

| Partie | Nombre de LED | Règle |
|---|---:|---|
| Segments rouges | 112 | 1 résistance par LED |
| Colon rouge | 2 | 1 résistance par LED |
| Contour rouge | 68 | 1 résistance par LED |
| Contour vert | 68 | 1 résistance par LED |
| Contour bleu | 68 | 1 résistance par LED |

> Les valeurs finales ne doivent pas être choisies uniquement par couleur. Elles seront calculées avec la tension d'alimentation, la tension directe `Vf`, le courant cible et le mode de fonctionnement. Pour le contour 12 V, une même valeur ne sera probablement pas utilisée pour rouge, vert et bleu.

## 18. Ordre de montage et de validation

| Étape | Ce qu'on câble | Validation |
|---:|---|---|
| 1 | Buck 12 V → 5 V sans charge | Mesurer exactement 5,0 V |
| 2 | Arduino + DS3231 | Lecture RTC |
| 3 | MAX7219 + 1 chiffre de 28 LED | Affichage test |
| 4 | 4 chiffres + colon | `00:00` / `88:88` |
| 5 | 74HC595 #1 + ULN2803A + 1 groupe contour | Groupe ON/OFF |
| 6 | Chaînage des 7 × 74HC595 | 51 groupes contrôlables |
| 7 | Contour rouge/vert/bleu complet | Animation |
| 8 | 4 boutons | Commandes |
| 9 | Buzzer | Sons |
| 10 | Animation de démarrage | Splash complet |
| 11 | Modification heure/minute | Mode pause |
| 12 | Tout le système | NOVAWATCH final |

**On valide chaque étape avant de passer à la suivante.**
