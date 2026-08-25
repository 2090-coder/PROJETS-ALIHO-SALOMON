# NOVAWATCH — Câblage précis V1

> Version orientée câblage : les connexions importantes sont présentées sous forme de tableaux « cette broche va vers cette broche ».

## 1. Arduino Nano — tableau principal

| Broche Arduino | Va vers | Fonction |
|---|---|---|
| D2 | Bouton 1 | ON/OFF |
| D3 | Bouton 2 | MODE / RESET / validation |
| D4 | Bouton 3 | + |
| D5 | Bouton 4 | - |
| D6 | Buzzer passif | Son |
| D7 | Les 2 LED du colon via résistances | `:` |
| D8 | SER du 74HC595 #1 | DATA contour |
| D9 | STCP des 74HC595 #1 à #7 | LATCH contour |
| D10 | LOAD/CS du MAX7219 | CS afficheur |
| D11 | DIN du MAX7219 | DATA afficheur |
| D12 | Libre | Extension future |
| D13 | CLK du MAX7219 + SHCP des 74HC595 | CLOCK partagé |
| A4 | SDA du DS3231 | I2C DATA |
| A5 | SCL du DS3231 | I2C CLOCK |
| 5V | Rail +5 V | Alimentation logique |
| GND | Rail GND | Masse commune |

## 2. MAX7219 DIP-24 — brochage précis

| Broche MAX7219 | Nom | Va vers | Fonction |
|---:|---|---|---|
| 1 | DIN | Arduino D11 | Données série |
| 2 | DIG0 | Cathode commune chiffre 1 | Dizaine des heures |
| 3 | DIG4 | Non connecté | Non utilisé |
| 4 | GND | GND commun | Masse |
| 5 | DIG6 | Non connecté | Non utilisé |
| 6 | DIG2 | Cathode commune chiffre 3 | Dizaine des minutes |
| 7 | DIG3 | Cathode commune chiffre 4 | Unité des minutes |
| 8 | DIG7 | Non connecté | Non utilisé |
| 9 | GND | GND commun | Masse |
| 10 | DIG5 | Non connecté | Non utilisé |
| 11 | DIG1 | Cathode commune chiffre 2 | Unité des heures |
| 12 | LOAD/CS | Arduino D10 | Validation |
| 13 | CLK | Arduino D13 | Horloge |
| 14 | SEG A | Anodes des LED du segment A via résistances | Segment A |
| 15 | SEG F | Anodes des LED du segment F via résistances | Segment F |
| 16 | SEG B | Anodes des LED du segment B via résistances | Segment B |
| 17 | SEG G | Anodes des LED du segment G via résistances | Segment G |
| 18 | ISET | RSET puis +5 V | Réglage du courant |
| 19 | V+ | +5 V | Alimentation |
| 20 | SEG C | Anodes des LED du segment C via résistances | Segment C |
| 21 | SEG E | Anodes des LED du segment E via résistances | Segment E |
| 22 | SEG DP | Non connecté | Pas de point décimal |
| 23 | SEG D | Anodes des LED du segment D via résistances | Segment D |
| 24 | DOUT | Non connecté en V1 | Extension éventuelle |

### Découplage MAX7219

| Composant | Va entre | Position |
|---|---|---|
| 100 nF | +5 V ↔ GND | Au plus près du MAX7219 |
| 10 µF | +5 V ↔ GND | Au plus près du MAX7219 |
| RSET | ISET ↔ +5 V | Valeur à valider selon le courant voulu |

## 3. Afficheur 7 segments

| Élément | Quantité |
|---|---:|
| LED par segment | 4 |
| Segments par chiffre | 7 |
| LED par chiffre | 28 |
| Chiffres | 4 |
| LED des segments | 112 |
| LED colon `:` | 2 |
| Total affichage | 114 |
| DP | Non utilisé |

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

| Sortie MAX7219 | Position |
|---|---|
| DIG0 | Dizaine heures |
| DIG1 | Unité heures |
| DIG2 | Dizaine minutes |
| DIG3 | Unité minutes |
| DIG4-DIG7 | Non utilisés |

## 4. Exemple : un segment composé de 4 LED

**Chaque LED possède sa propre résistance.**

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
| Cathodes LED A1-A4 | Cathode commune du chiffre → DIG correspondant |

```text
SEG A
  |
  +--- R_A1 --- LED A1 ---+
  +--- R_A2 --- LED A2 ---+
  +--- R_A3 --- LED A3 ---+---- DIGx
  +--- R_A4 --- LED A4 ---+
```

Même principe pour les 7 segments et les 4 chiffres.

## 5. Colon `:`

| Arduino | Va vers | Puis vers |
|---|---|---|
| D7 | Résistance colon 1 | LED colon haut → GND |
| D7 | Résistance colon 2 | LED colon bas → GND |

Le colon n'utilise pas SEG DP.

## 6. Contour — LED séparées par couleur

| Couleur | LED 3 mm | Groupes de 4 |
|---|---:|---:|
| Rouge | 68 | 17 |
| Vert | 68 | 17 |
| Bleu | 68 | 17 |
| Total | 204 | 51 |

Il ne s'agit pas de LED RGB intégrées : chaque LED a une seule couleur.

## 7. Groupe contour de 4 LED

| Élément | Va vers |
|---|---|
| +5 V | Résistance R1 |
| Résistance R1 | Anode LED 1 |
| +5 V | Résistance R2 |
| Résistance R2 | Anode LED 2 |
| +5 V | Résistance R3 |
| Résistance R3 | Anode LED 3 |
| +5 V | Résistance R4 |
| Résistance R4 | Anode LED 4 |
| Cathodes LED 1-4 | Sortie OUT du ULN2803A |

## 8. 74HC595 → ULN2803A → contour

| 74HC595 | Sortie | ULN2803A | Entrée | Groupe |
|---|---|---|---|---|
| #1 | Q0 | #1 | IN1 | Rouge 01 |
| #1 | Q1 | #1 | IN2 | Rouge 02 |
| #1 | Q2 | #1 | IN3 | Rouge 03 |
| #1 | Q3 | #1 | IN4 | Rouge 04 |
| #1 | Q4 | #1 | IN5 | Rouge 05 |
| #1 | Q5 | #1 | IN6 | Rouge 06 |
| #1 | Q6 | #1 | IN7 | Rouge 07 |
| #1 | Q7 | #1 | IN8 | Rouge 08 |
| #2 | Q0 | #2 | IN1 | Rouge 09 |
| #2 | Q1 | #2 | IN2 | Rouge 10 |
| #2 | Q2 | #2 | IN3 | Rouge 11 |
| #2 | Q3 | #2 | IN4 | Rouge 12 |
| #2 | Q4 | #2 | IN5 | Rouge 13 |
| #2 | Q5 | #2 | IN6 | Rouge 14 |
| #2 | Q6 | #2 | IN7 | Rouge 15 |
| #2 | Q7 | #2 | IN8 | Rouge 16 |
| #3 | Q0 | #3 | IN1 | Rouge 17 |
| #3 | Q1 | #3 | IN2 | Vert 01 |
| #3 | Q2 | #3 | IN3 | Vert 02 |
| #3 | Q3 | #3 | IN4 | Vert 03 |
| #3 | Q4 | #3 | IN5 | Vert 04 |
| #3 | Q5 | #3 | IN6 | Vert 05 |
| #3 | Q6 | #3 | IN7 | Vert 06 |
| #3 | Q7 | #3 | IN8 | Vert 07 |
| #4 | Q0 | #4 | IN1 | Vert 08 |
| #4 | Q1 | #4 | IN2 | Vert 09 |
| #4 | Q2 | #4 | IN3 | Vert 10 |
| #4 | Q3 | #4 | IN4 | Vert 11 |
| #4 | Q4 | #4 | IN5 | Vert 12 |
| #4 | Q5 | #4 | IN6 | Vert 13 |
| #4 | Q6 | #4 | IN7 | Vert 14 |
| #4 | Q7 | #4 | IN8 | Vert 15 |
| #5 | Q0 | #5 | IN1 | Vert 16 |
| #5 | Q1 | #5 | IN2 | Vert 17 |
| #5 | Q2 | #5 | IN3 | Bleu 01 |
| #5 | Q3 | #5 | IN4 | Bleu 02 |
| #5 | Q4 | #5 | IN5 | Bleu 03 |
| #5 | Q5 | #5 | IN6 | Bleu 04 |
| #5 | Q6 | #5 | IN7 | Bleu 05 |
| #5 | Q7 | #5 | IN8 | Bleu 06 |
| #6 | Q0 | #6 | IN1 | Bleu 07 |
| #6 | Q1 | #6 | IN2 | Bleu 08 |
| #6 | Q2 | #6 | IN3 | Bleu 09 |
| #6 | Q3 | #6 | IN4 | Bleu 10 |
| #6 | Q4 | #6 | IN5 | Bleu 11 |
| #6 | Q5 | #6 | IN6 | Bleu 12 |
| #6 | Q6 | #6 | IN7 | Bleu 13 |
| #6 | Q7 | #6 | IN8 | Bleu 14 |
| #7 | Q0 | #7 | IN1 | Bleu 15 |
| #7 | Q1 | #7 | IN2 | Bleu 16 |
| #7 | Q2 | #7 | IN3 | Bleu 17 |
| #7 | Q3-Q7 | — | — | Réservées |

## 9. Chaînage des 74HC595

| Source | Va vers | Fonction |
|---|---|---|
| Arduino D8 | SER #1 | DATA |
| Q7S #1 | SER #2 | Chaînage |
| Q7S #2 | SER #3 | Chaînage |
| Q7S #3 | SER #4 | Chaînage |
| Q7S #4 | SER #5 | Chaînage |
| Q7S #5 | SER #6 | Chaînage |
| Q7S #6 | SER #7 | Chaînage |
| Arduino D13 | SHCP #1 à #7 | CLOCK |
| Arduino D9 | STCP #1 à #7 | LATCH |
| +5 V | VCC #1 à #7 | Alimentation |
| GND | GND #1 à #7 | Masse |
| +5 V | MR/SRCLR #1 à #7 | Reset désactivé |
| GND | OE #1 à #7 | Sorties activées |

## 10. DS3231

| Arduino Nano | DS3231 | Fonction |
|---|---|---|
| A4 | SDA | I2C DATA |
| A5 | SCL | I2C CLOCK |
| 5V | VCC | Alimentation |
| GND | GND | Masse |

## 11. Boutons

| Bouton | Broche Arduino | Fonction |
|---|---|---|
| Bouton 1 | D2 | ON/OFF |
| Bouton 2 | D3 | RESET / MODE / validation |
| Bouton 3 | D4 | + |
| Bouton 4 | D5 | - |

Pour le premier prototype : bouton entre broche Arduino et GND, avec `INPUT_PULLUP`.

## 12. Buzzer

| Arduino | Va vers | Fonction |
|---|---|---|
| D6 | Buzzer passif | Son |
| GND | Buzzer | Masse |

## 13. Alimentation

| Élément | Va vers |
|---|---|
| Prise JACK | Entrée alimentation |
| Alimentation/buck | +5 V régulé |
| +5 V principal | Arduino + MAX7219 + DS3231 + 74HC595 + LEDs |
| GND principal | Masse commune |

Prévoir au minimum une alimentation 5 V régulée de 3 A pour le prototype, avec marge selon les groupes du contour allumés.

**Ne pas alimenter les 204 LED du contour depuis le régulateur du Nano.**

## 14. Résistances

| Partie | Nombre de LED | Règle |
|---|---:|---|
| Segments | 112 | 1 résistance par LED |
| Colon | 2 | 1 résistance par LED |
| Contour rouge | 68 | 1 résistance par LED |
| Contour vert | 68 | 1 résistance par LED |
| Contour bleu | 68 | 1 résistance par LED |

Pour les premiers essais du contour à 5 V, 680 Ω est une valeur de départ prudente. La valeur finale doit être calculée avec la tension directe et le courant cible des LED réelles. La valeur RSET du MAX7219 sera fixée après validation du courant voulu.

## 15. Ordre de montage

| Étape | Ce qu'on câble | Validation |
|---:|---|---|
| 1 | 4 LED + 4 résistances = 1 segment | Segment OK |
| 2 | 28 LED = 1 chiffre `8` | 7 segments OK |
| 3 | 1 chiffre + MAX7219 | Multiplexage OK |
| 4 | 4 chiffres + colon | `00:00` / `88:88` |
| 5 | DS3231 | Heure OK |
| 6 | 4 boutons | Commandes OK |
| 7 | 1 groupe rouge contour | Driver OK |
| 8 | 1 groupe vert + 1 bleu | Couleurs OK |
| 9 | 51 groupes contour | Animation OK |
| 10 | Buzzer | Sons OK |
| 11 | Tout le système | NOVAWATCH complet |

**On valide chaque étape avant de passer à la suivante.**
