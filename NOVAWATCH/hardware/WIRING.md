# NOVAWATCH - Cablage HT16K33

Ce document remplace l ancien cablage MAX7219.

## 1. Architecture

| Element | Quantite | Role |
|---|---:|---|
| Arduino Nano | 1 | controleur |
| Module HT16K33 | 1 | multiplexage affichage |
| DS3231 | 1 | horloge temps reel |
| 74HC595 | 7 | commande contour |
| ULN2803A | 7 | commutation contour |
| Afficheur 4 chiffres cathode commune | 1 | HH:MM |
| LED deux points | 2 | separation heures/minutes |
| LED contour | 160 | 40 groupes de 4 |
| Boutons | 4 | commandes |
| Avertisseur passif | 1 | sons |

## 2. Alimentation

Entree principale : 12 V DC.

Le 12 V alimente le contour selon son cablage.

Un convertisseur abaisseur doit fournir un rail stable de 5 V pour la logique.

| Source | Connexion |
|---|---|
| +12 V | entree du convertisseur abaisseur |
| GND 12 V | GND commun |
| sortie +5 V | Arduino et circuits logiques |
| sortie GND | GND commun |

Ne jamais envoyer 12 V sur la broche 5V du Nano.

Avant de brancher le Nano, regler et mesurer le convertisseur a 5,0 V.

## 3. Arduino Nano

| Broche Nano | Connexion | Fonction |
|---|---|---|
| D2 | bouton vers GND | alimentation |
| D3 | bouton vers GND | mode |
| D4 | bouton vers GND | plus |
| D5 | bouton vers GND | moins |
| D6 | avertisseur passif | son |
| D7 | 2 LED avec resistances | deux points |
| D8 | DS du 595 #1 | donnees 595 |
| D9 | STCP de tous les 595 | verrou |
| D13 | SHCP de tous les 595 | horloge |
| A4 | SDA DS3231 + SDA HT16K33 | SDA |
| A5 | SCL DS3231 + SCL HT16K33 | SCL |
| 5V | rail +5 V | alimentation |
| GND | rail GND | masse |

D10 et D11 ne sont plus utilises pour l affichage.

## 4. Module HT16K33

| Module | Arduino Nano |
|---|---|
| VDD | +5 V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

Adresse utilisee : 0x70.

Si le module possede des cavaliers ou pastilles A0/A1/A2, ils doivent rester dans la configuration correspondant a 0x70.

## 5. Bus I2C partage

Les deux circuits sont relies en parallele :

```text
Nano A4 SDA -------- DS3231 SDA
       |
       +------------ HT16K33 SDA

Nano A5 SCL -------- DS3231 SCL
       |
       +------------ HT16K33 SCL

Nano 5V  ----------- DS3231 VCC
       |
       +------------ HT16K33 VDD

Nano GND ----------- DS3231 GND
       |
       +------------ HT16K33 GND
```

Adresses :

- DS3231 = 0x68
- HT16K33 = 0x70

Elles sont differentes.

## 6. Correspondance HT16K33

Le HT16K33 possede ROW0 a ROW15 et COM0 a COM7.

Pour le panneau 4 chiffres :

| Sortie HT16K33 | Fonction NOVAWATCH |
|---|---|
| ROW0 | segment A |
| ROW1 | segment B |
| ROW2 | segment C |
| ROW3 | segment D |
| ROW4 | segment E |
| ROW5 | segment F |
| ROW6 | segment G |
| ROW7 | point decimal non utilise |
| COM0 | cathode commune chiffre 1 |
| COM1 | cathode commune chiffre 2 |
| COM2 | cathode commune chiffre 3 |
| COM3 | cathode commune chiffre 4 |

Les sorties ROW8 a ROW15 et COM4 a COM7 restent inutilisees.

### Correspondance RAM

| COM | Adresse RAM ROW0-ROW7 |
|---|---:|
| COM0 | 0x00 |
| COM1 | 0x02 |
| COM2 | 0x04 |
| COM3 | 0x06 |

Cette correspondance vient de la documentation officielle HT16K33.

## 7. Module repere A0..A15 et C0..C7

Si le module utilise les reperes :

- A0..A15 ;
- C0..C7 ;

la correspondance logique du projet est :

| Module | Fonction |
|---|---|
| A0 | ROW0 = segment A |
| A1 | ROW1 = segment B |
| A2 | ROW2 = segment C |
| A3 | ROW3 = segment D |
| A4 | ROW4 = segment E |
| A5 | ROW5 = segment F |
| A6 | ROW6 = segment G |
| A7 | ROW7 = point decimal non utilise |
| C0 | COM0 = chiffre 1 |
| C1 | COM1 = chiffre 2 |
| C2 | COM2 = chiffre 3 |
| C3 | COM3 = chiffre 4 |

Attention : A0..A15 et C0..C7 sont des reperes du module. Ils ne donnent pas automatiquement les numeros physiques des broches de ton afficheur 4 chiffres.

Pour le brochage physique exact de l afficheur, utiliser sa reference ou son schema. Ne pas deviner les numeros des 12 broches.

## 8. Afficheur 4 chiffres cathode commune

Le projet utilise un afficheur 4 chiffres a cathode commune.

La logique est :

```text
ROW0 -> segment A
ROW1 -> segment B
ROW2 -> segment C
ROW3 -> segment D
ROW4 -> segment E
ROW5 -> segment F
ROW6 -> segment G

COM0 -> chiffre 1
COM1 -> chiffre 2
COM2 -> chiffre 3
COM3 -> chiffre 4
```

Les anodes des segments vont vers les sorties ROW selon le brochage reel du panneau.

Les cathodes communes vont vers COM selon le brochage reel du panneau.

Le HT16K33 est un pilote multiplexe : il ne faut pas traiter ce montage comme 4 afficheurs independants.

## 9. Attention aux resistances des LED

Le panneau utilise plusieurs LED par segment dans la conception NOVAWATCH.

Chaque LED doit avoir une resistance adaptee si plusieurs LED sont utilisees en parallele.

Ne pas mettre plusieurs LED nues en parallele sur une meme sortie.

La valeur des resistances doit etre determinee selon :

- tension d alimentation ;
- tension directe de la LED ;
- courant souhaite ;
- courant admissible par le HT16K33 ;
- nombre de LED actives pendant le multiplexage.

Valider le courant au multimetre avant le fonctionnement prolonge.

## 10. Deux points

Les deux points utilisent D7.

Chaque LED doit avoir sa propre resistance.

```text
D7 -> resistance -> anode LED haut
D7 -> resistance -> anode LED bas

cathode LED haut -> GND
cathode LED bas  -> GND
```

Une valeur de depart de 470 ohms peut etre utilisee pour un premier essai, puis ajustee apres mesure.

## 11. 74HC595

Pour chaque 74HC595 :

| Broche | Nom | Connexion |
|---:|---|---|
| 1 | Q1 | entree ULN |
| 2 | Q2 | entree ULN |
| 3 | Q3 | entree ULN |
| 4 | Q4 | entree ULN |
| 5 | Q5 | entree ULN |
| 6 | Q6 | entree ULN |
| 7 | Q7 | entree ULN |
| 8 | GND | GND |
| 9 | Q7S | DS du 595 suivant |
| 10 | MR | +5 V |
| 11 | SHCP | D13 |
| 12 | STCP | D9 |
| 13 | OE | GND |
| 14 | DS | D8 ou Q7S precedent |
| 15 | Q0 | entree ULN |
| 16 | VCC | +5 V |

## 12. Chaine des 74HC595

```text
D8 -> DS #1
Q7S #1 -> DS #2
Q7S #2 -> DS #3
Q7S #3 -> DS #4
Q7S #4 -> DS #5
Q7S #5 -> DS #6
Q7S #6 -> DS #7
```

Tous les SHCP sont relies a D13.

Tous les STCP sont relies a D9.

Tous les OE sont relies a GND.

Tous les MR sont relies a +5 V.

Le code envoie le dernier 595 en premier pour conserver la correspondance des groupes.

## 13. 40 groupes du contour

Le projet utilise 40 sorties :

- 595 #1 : G01 a G08
- 595 #2 : G09 a G16
- 595 #3 : G17 a G24
- 595 #4 : G25 a G32
- 595 #5 : G33 a G40
- 595 #6 : inutilise
- 595 #7 : inutilise

Chaque groupe contient 4 LED.

Total contour : 40 x 4 = 160 LED.

## 14. ULN2803A

Chaque sortie d un 74HC595 va vers une entree du ULN2803A.

Le ULN2803A commute la masse du groupe.

Il ne fournit pas le +12 V.

Le +12 V va vers les anodes des LED avec resistances adaptees.

Les cathodes des LED du groupe vont vers la sortie correspondante du ULN.

## 15. Verification avant mise sous tension

1. Mesurer le 5 V.
2. Verifier le GND commun.
3. Verifier SDA et SCL.
4. Verifier que DS3231 est a 0x68.
5. Verifier que HT16K33 est a 0x70.
6. Verifier le sens de la chaine des 595.
7. Verifier D8, D9 et D13.
8. Verifier les 40 groupes.
9. Verifier une resistance par LED lorsque le montage l exige.
10. Tester d abord le HT16K33 avec un seul chiffre.
11. Tester ensuite les quatre chiffres.
12. Tester ensuite le contour.
13. Tester enfin le montage complet.

## 16. Reference officielle

Documentation Holtek HT16K33A :

https://www.holtek.com/webapi/116711/HT16K33Av110.pdf

La documentation indique notamment :

- 16 sorties ROW ;
- 8 sorties COM ;
- RAM 16 x 8 ;
- COM0 a l adresse 0x00/0x01 ;
- COM1 a 0x02/0x03 ;
- COM2 a 0x04/0x05 ;
- COM3 a 0x06/0x07.
