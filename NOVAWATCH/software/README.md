# NOVAWATCH - Logiciel

## Carte

- Arduino Nano
- HT16K33
- DS3231
- 7 x 74HC595
- 7 x ULN2803A
- avertisseur passif

## Fichier principal

`NOVAWATCH.ino`

## Fonctionnalites

- activation et arret par bouton ;
- animation de demarrage ;
- melodie de demarrage ;
- affichage HH:MM avec HT16K33 ;
- lecture de l heure avec DS3231 ;
- animation du contour ;
- 40 groupes de contour ;
- 4 LED par groupe ;
- 160 LED de contour ;
- reglage des heures ;
- reglage des minutes ;
- clignotement du champ en cours ;
- son different selon le chiffre modifie ;
- utilisation de millis() pour eviter les attentes bloquantes ;
- verification du bus I2C au demarrage.

## Noms techniques HT16K33

Le code garde les noms techniques du composant lorsqu ils sont necessaires :

- ROW0 a ROW15 ;
- COM0 a COM7 ;
- RAM ;
- adresse I2C.

Les variables et fonctions du programme sont en francais sans accents.

## I2C

- DS3231 : 0x68
- HT16K33 : 0x70

Le Nano utilise A4 pour SDA et A5 pour SCL.

## Affichage

La correspondance logique utilisee est :

- ROW0 = A
- ROW1 = B
- ROW2 = C
- ROW3 = D
- ROW4 = E
- ROW5 = F
- ROW6 = G
- COM0 = chiffre 1
- COM1 = chiffre 2
- COM2 = chiffre 3
- COM3 = chiffre 4

Le point decimal n est pas utilise.

## Important

Le programme suppose que le cablage reel respecte :

`../hardware/WIRING.md`.
