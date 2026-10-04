# NOVAWATCH

## Objectif

NOVAWATCH est une horloge numerique artisanale basee sur un Arduino Nano.

Le systeme utilise :

- un Arduino Nano ;
- un HT16K33 pour le multiplexage de l affichage 7 segments ;
- un DS3231 pour conserver l heure ;
- 7 circuits 74HC595 ;
- 7 circuits ULN2803A ;
- 4 boutons ;
- un avertisseur passif ;
- 2 LED pour les deux points ;
- un contour de 40 groupes de 4 LED.

Le projet est concu pour du vrai materiel. Il ne depend pas d une simulation.

## Architecture

```text
                         +-------------------+
                         |    ARDUINO NANO   |
                         +---------+---------+
                                   |
              +--------------------+--------------------+
              |                    |                    |
              v                    v                    v
        +-----------+        +-----------+        +-----------+
        | HT16K33   |        |  DS3231   |        | 7x595     |
        | I2C 0x70  |        | I2C 0x68  |        | + 7xULN   |
        +-----+-----+        +-----------+        +-----+-----+
              |                                      |
              v                                      v
       4 chiffres 7 segments                  40 groupes contour
              |                                      |
        + 2 LED deux points                         160 LED
```

## Bus I2C

Le Nano utilise :

- A4 = SDA
- A5 = SCL

Les deux circuits partagent le meme bus :

- DS3231 = 0x68
- HT16K33 = 0x70

Les adresses ne sont donc pas en conflit.

## HT16K33

Le HT16K33 possede 16 sorties ROW et 8 sorties COM.

Pour NOVAWATCH :

| HT16K33 | Fonction |
|---|---|
| ROW0 | segment A |
| ROW1 | segment B |
| ROW2 | segment C |
| ROW3 | segment D |
| ROW4 | segment E |
| ROW5 | segment F |
| ROW6 | segment G |
| ROW7 | point decimal non utilise |
| COM0 | chiffre 1 |
| COM1 | chiffre 2 |
| COM2 | chiffre 3 |
| COM3 | chiffre 4 |

La RAM utilisee est :

| COM | Adresse ROW0-ROW7 |
|---|---:|
| COM0 | 0x00 |
| COM1 | 0x02 |
| COM2 | 0x04 |
| COM3 | 0x06 |

Le code utilise directement le bus I2C avec `Wire.h`. Aucune bibliotheque HT16K33 externe n est necessaire.

## Attention au module HT16K33

Certains modules utilisent des reperes comme A0..A15 et C0..C7.

Ces reperes doivent etre lus sur la serigraphie du module. Ne pas deviner les numeros physiques du connecteur du panneau 4 chiffres sans connaitre le brochage exact du panneau.

## Broches Arduino

| Nano | Fonction |
|---|---|
| D2 | bouton alimentation |
| D3 | bouton mode |
| D4 | bouton plus |
| D5 | bouton moins |
| D6 | avertisseur passif |
| D7 | deux points |
| D8 | DS des 74HC595 |
| D9 | STCP des 74HC595 |
| D13 | SHCP des 74HC595 |
| A4 | SDA |
| A5 | SCL |

D10 et D11 ne sont plus utilises pour l affichage.

## Commandes

### Hors mode reglage

- 1 clic sur MODE : remise de l horloge a 00:00:00
- 2 clics sur MODE : entree dans le reglage
- PLUS et MOINS ne sont actifs qu en mode reglage

### Mode reglage

- 1 clic sur MODE : passer des heures aux minutes ou inversement
- PLUS : augmenter la valeur
- MOINS : diminuer la valeur
- 3 clics sur MODE : enregistrer et sortir

Le champ en cours clignote.

## Demarrage

Lors de l activation :

1. animation des segments ;
2. melodie ;
3. animation du contour ;
4. lecture du DS3231 ;
5. affichage de HH:MM.

## Contour

Le contour utilise :

- 7 x 74HC595 ;
- 56 sorties disponibles ;
- 40 sorties utilisees ;
- 16 sorties non utilisees ;
- 40 groupes ;
- 4 LED par groupe ;
- 160 LED au total.

Les ULN2803A commandent les masses des groupes.

La couleur des LED depend du cablage reel. Le logiciel ne genere pas de couleur RGB.

## Alimentation

Le projet recoit une alimentation principale de 12 V DC.

Un convertisseur abaisseur doit fournir le 5 V pour :

- Arduino Nano ;
- HT16K33 ;
- DS3231 ;
- 74HC595 ;
- logique des ULN2803A.

Ne jamais appliquer directement 12 V sur la ligne 5 V.

## Fichiers

- `software/NOVAWATCH.ino` : programme principal
- `software/README.md` : documentation du logiciel
- `hardware/WIRING.md` : cablage et validation materielle

## Reference HT16K33

Documentation officielle Holtek :

https://www.holtek.com/webapi/116711/HT16K33Av110.pdf
