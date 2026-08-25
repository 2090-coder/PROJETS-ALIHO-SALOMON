# NOVAWATCH

## 1. Objectif

NOVAWATCH est une horloge numérique artisanale affichant `HH:MM` avec quatre chiffres 7 segments construits avec des LED rouges 3 mm.

Chaque chiffre possède 7 segments et chaque segment contient 4 LED rouges :

- 4 chiffres × 7 segments × 4 LED = **112 LED rouges**
- 2 LED rouges pour les deux points centraux `:`
- Total affichage = **114 LED rouges**
- Aucun point décimal (DP)

Le contour est constitué de LED individuelles 3 mm :

- 68 LED rouges
- 68 LED vertes
- 68 LED bleues
- soit **204 LED de contour**
- organisation retenue : 17 groupes de 4 LED par couleur

## 2. Architecture matérielle retenue

```text
                         +----------------+
                         |  ARDUINO NANO  |
                         +--------+-------+
                                  |
             +--------------------+--------------------+
             |                    |                    |
             v                    v                    v
        MAX7219 +            DS3231 RTC          74HC595 x7
       affichage HH:MM            I2C             + ULN2803A x7
             |                                         |
             v                                         v
      112 LED segments                         204 LED contour
        + 2 LED colon                       R / V / B, groupes de 4

             Arduino Nano
                  |
             Buzzer passif
                  |
             4 boutons
```

## 3. Principe important

Les 112 LED des chiffres ne sont pas 112 sorties indépendantes. Les 4 LED d'un même segment sont regroupées électriquement dans une même position de segment, avec une résistance individuelle par LED. Le MAX7219 assure le multiplexage des 4 chiffres.

Le contour n'est pas constitué de LED RGB intégrées. Chaque LED est une LED indépendante de sa couleur. Les groupes de 4 LED d'une même couleur sont commandés par les sorties des ULN2803A.

## 4. Première règle de construction

Ne pas fabriquer les 114 LED de l'afficheur et les 204 LED du contour avant validation du premier sous-ensemble. Le premier test sera un chiffre 7 segments complet avec 4 LED par segment.

Voir `NOVAWATCH/hardware/WIRING.md` pour le câblage.
