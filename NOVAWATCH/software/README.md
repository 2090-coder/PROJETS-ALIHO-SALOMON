# NOVAWATCH — Software V1

## Carte

- Arduino Nano
- MAX7219
- DS3231
- 7 x 74HC595
- 7 x ULN2803A
- Buzzer passif

## Fichier principal

`NOVAWATCH.ino`

## Fonctionnalités V1

- ON/OFF par bouton 1
- Animation SPLASH au démarrage
- Mélodie de démarrage
- Affichage HH:MM avec MAX7219
- Lecture de l'heure avec DS3231
- Animation permanente du contour
- **40 groupes de contour x 4 LED = 160 LED**
- Les groupes gardent la couleur définie par leur câblage physique
- Bouton 2 : 1 clic = reset, 2 clics = entrée en modification
- En modification : heures clignotantes en premier
- Bouton 2 : 1 clic = passage heures/minutes
- Bouton 2 : 3 clics = validation et sortie
- Bouton 3 : +
- Bouton 4 : -
- Son différent selon le chiffre modifié
- Utilisation de `millis()` pour éviter les `delay()` bloquants

## Contour

Le câblage conserve 7 x 74HC595, soit 56 sorties disponibles. NOVAWATCH utilise actuellement 40 sorties pour les 40 groupes du contour. Les 16 sorties restantes sont laissées inutilisées.

Chaque groupe commande 4 LED de la même couleur via le ULN2803A correspondant. La couleur n'est pas créée par le logiciel : elle dépend de la couleur des LED réellement câblées sur le groupe.

## Important

Le code suppose que le câblage matériel suit `../hardware/WIRING.md`.

L'alimentation générale du prototype est prévue en 12 V DC, avec conversion 12 V vers 5 V pour la logique. Les LED du contour sont pilotées sous 12 V par les ULN2803A.
