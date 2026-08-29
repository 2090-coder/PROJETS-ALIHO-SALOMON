# NOVAWATCH — Câblage précis V2 FINAL

Ce document est la référence matérielle du prototype NOVAWATCH.

## 1. Architecture générale

| Élément | Quantité | Alimentation | Rôle |
|---|---:|---:|---|
| Arduino Nano | 1 | 5 V | Contrôleur |
| MAX7219 DIP-24 | 1 | 5 V | Multiplexage des 4 chiffres |
| DS3231 | 1 | 5 V | Horloge temps réel |
| 74HC595 DIP-16 | 7 | 5 V | Commande du contour |
| ULN2803A DIP-18 | 7 | logique 5 V | Commutation des groupes LED |
| LED rouge 3 mm affichage | 112 | via MAX7219 | 4 LED par segment |
| LED rouge 3 mm colon | 2 | via D7 | Séparateur heures/minutes |
| LED contour | 160 | 12 V | 40 groupes de 4 |
| Boutons poussoirs | 4 | logique | ON/OFF, MODE/RESET, +, - |
| Buzzer passif | 1 | logique | Sons |
| Prise JACK DC | 1 | 12 V | Entrée alimentation |
| Buck 12 V → 5 V | 1 | 12 V entrée | Rail logique 5 V |

**Total LED : 112 + 2 + 160 = 274 LED.**

Le contour est composé de **40 groupes × 4 LED = 160 LED**. Les LED du contour sont des LED séparées par couleur, jamais des LED RGB intégrées.

---

## 2. Alimentation 12 V DC

| Source | Va vers | Tension |
|---|---|---:|
| JACK + | Fusible/interrupteur | +12 V |
| Fusible/interrupteur | Entrée + du buck | +12 V |
| Fusible/interrupteur | Anodes des LED contour via résistances | +12 V |
| JACK - | GND commun | 0 V |
| Buck sortie + | Rail logique | +5 V |
| Buck sortie - | GND commun | 0 V |

Le buck doit être réglé à **5,0 V avant branchement** de l'Arduino et des CI.

Le GND du 12 V, le GND du buck, l'Arduino, le MAX7219, les 74HC595, les ULN2803A et le DS3231 doivent être communs.

**Ne jamais envoyer 12 V sur la broche 5V de l'Arduino ni sur VCC des CI.**

---

## 3. Arduino Nano

| Broche Nano | Va vers | Fonction |
|---|---|---|
| D2 | Bouton 1, autre contact → GND | ON/OFF |
| D3 | Bouton 2, autre contact → GND | RESET / MODE / validation |
| D4 | Bouton 3, autre contact → GND | + |
| D5 | Bouton 4, autre contact → GND | - |
| D6 | Buzzer passif + | Buzzer |
| D7 | Deux résistances séparées → 2 LED colon | `:` |
| D8 | DS pin 14 du 74HC595 #1 | DATA contour |
| D9 | STCP pin 12 des 7 × 74HC595 | LATCH contour |
| D10 | LOAD/CS pin 12 MAX7219 | CS afficheur |
| D11 | DIN pin 1 MAX7219 | DATA afficheur |
| D12 | Libre | Réserve |
| D13 | CLK pin 13 MAX7219 + SHCP pin 11 des 7 × 74HC595 | CLOCK partagé |
| A4 | SDA DS3231 | I²C SDA |
| A5 | SCL DS3231 | I²C SCL |
| 5V | Rail +5 V | Alimentation logique |
| GND | Rail GND | Masse commune |

Les boutons utilisent `INPUT_PULLUP` : au repos = HIGH, appui = LOW.

---

## 4. MAX7219 DIP-24

| Pin | Nom réel | Connexion |
|---:|---|---|
| 1 | DIN | Arduino D11 |
| 2 | DIG0 | Cathodes communes chiffre 1 |
| 3 | DIG4 | NC |
| 4 | GND | GND |
| 5 | DIG6 | NC |
| 6 | DIG2 | Cathodes communes chiffre 3 |
| 7 | DIG3 | Cathodes communes chiffre 4 |
| 8 | DIG7 | NC |
| 9 | GND | GND |
| 10 | DIG5 | NC |
| 11 | DIG1 | Cathodes communes chiffre 2 |
| 12 | LOAD/CS | Arduino D10 |
| 13 | CLK | Arduino D13 |
| 14 | SEG A | Anodes segment A via résistances |
| 15 | SEG F | Anodes segment F via résistances |
| 16 | SEG B | Anodes segment B via résistances |
| 17 | SEG G | Anodes segment G via résistances |
| 18 | ISET | RSET vers +5 V |
| 19 | V+ | +5 V |
| 20 | SEG C | Anodes segment C via résistances |
| 21 | SEG E | Anodes segment E via résistances |
| 22 | SEG DP | NC |
| 23 | SEG D | Anodes segment D via résistances |
| 24 | DOUT | NC |

### MAX7219 — composants associés

| Composant | Connexion |
|---|---|
| 100 nF | entre V+ et GND, au plus près du MAX7219 |
| 10 µF | entre V+ et GND, au plus près du MAX7219 |
| RSET | ISET pin 18 → résistance → +5 V |

Pour ce prototype avec 4 LED par segment, une valeur de départ raisonnable est **RSET = 9,53 kΩ 1 %**, à valider par mesure et selon la luminosité souhaitée. Ne pas supprimer RSET.

---

## 5. Affichage 7 segments artisanal

| Élément | Quantité |
|---|---:|
| Chiffres | 4 |
| Segments par chiffre | 7 |
| LED rouges par segment | 4 |
| LED par chiffre | 28 |
| LED segments | 112 |
| LED colon | 2 |
| Total affichage | **114** |
| DP | **Non utilisé** |

### Correspondance

| MAX7219 | Segment physique |
|---|---|
| SEG A | supérieur |
| SEG B | supérieur droit |
| SEG C | inférieur droit |
| SEG D | inférieur |
| SEG E | inférieur gauche |
| SEG F | supérieur gauche |
| SEG G | central |

| MAX7219 | Chiffre |
|---|---|
| DIG0 | dizaine heure |
| DIG1 | unité heure |
| DIG2 | dizaine minute |
| DIG3 | unité minute |

### Chaque segment

Chaque LED possède **sa propre résistance**.

Exemple pour le segment A du chiffre 1 :

| Départ | Va vers |
|---|---|
| MAX7219 SEG A | Résistance A1 |
| Résistance A1 | Anode LED A1 |
| MAX7219 SEG A | Résistance A2 |
| Résistance A2 | Anode LED A2 |
| MAX7219 SEG A | Résistance A3 |
| Résistance A3 | Anode LED A3 |
| MAX7219 SEG A | Résistance A4 |
| Résistance A4 | Anode LED A4 |
| Cathodes LED A1-A4 | DIG0 |

Même principe pour A-G des quatre chiffres. Ne pas mettre une seule résistance commune aux quatre LED d'un segment.

---

## 6. Colon `:`

Le colon n'utilise pas SEG DP.

| Départ | Va vers |
|---|---|
| Arduino D7 | Résistance colon haut → anode LED haut |
| Arduino D7 | Résistance colon bas → anode LED bas |
| Cathode LED colon haut | GND |
| Cathode LED colon bas | GND |

Utiliser une résistance par LED, par exemple **470 Ω** pour commencer avec une alimentation logique 5 V.

---

## 7. 74HC595 — nomenclature réelle

Pour chaque 74HC595 :

| Pin | Nom réel | Connexion |
|---:|---|---|
| 1 | Q1 | entrée ULN correspondante |
| 2 | Q2 | entrée ULN correspondante |
| 3 | Q3 | entrée ULN correspondante |
| 4 | Q4 | entrée ULN correspondante |
| 5 | Q5 | entrée ULN correspondante |
| 6 | Q6 | entrée ULN correspondante |
| 7 | Q7 | entrée ULN correspondante |
| 8 | GND | GND |
| 9 | Q7S | DS du 595 suivant |
| 10 | MR | +5 V |
| 11 | SHCP | Arduino D13 |
| 12 | STCP | Arduino D9 |
| 13 | OE | GND |
| 14 | DS | Arduino D8 ou Q7S précédent |
| 15 | Q0 | entrée ULN correspondante |
| 16 | VCC | +5 V |

### Chaînage

| Départ | Va vers |
|---|---|
| D8 | DS pin 14 du #1 |
| Q7S pin 9 #1 | DS pin 14 #2 |
| Q7S pin 9 #2 | DS pin 14 #3 |
| Q7S pin 9 #3 | DS pin 14 #4 |
| Q7S pin 9 #4 | DS pin 14 #5 |
| Q7S pin 9 #5 | DS pin 14 #6 |
| Q7S pin 9 #6 | DS pin 14 #7 |
| Q7S pin 9 #7 | NC |

Pour les 7 CI : pin 8 → GND, pin 10 → +5 V, pin 11 → D13, pin 12 → D9, pin 13 → GND, pin 16 → +5 V.

---

## 8. 74HC595 → ULN2803A → 40 groupes

Le contour contient **40 groupes exactement**. Chaque groupe contient 4 LED. Les 16 sorties restantes des 7 registres ne sont pas utilisées.

| 74HC595 | Sortie | Pin | ULN2803A | Entrée | Groupe contour |
|---|---|---:|---|---:|---:|
| #1 | Q0 | 15 | #1 | IN1 | G01 |
| #1 | Q1 | 1 | #1 | IN2 | G02 |
| #1 | Q2 | 2 | #1 | IN3 | G03 |
| #1 | Q3 | 3 | #1 | IN4 | G04 |
| #1 | Q4 | 4 | #1 | IN5 | G05 |
| #1 | Q5 | 5 | #1 | IN6 | G06 |
| #1 | Q6 | 6 | #1 | IN7 | G07 |
| #1 | Q7 | 7 | #1 | IN8 | G08 |
| #2 | Q0 | 15 | #2 | IN1 | G09 |
| #2 | Q1 | 1 | #2 | IN2 | G10 |
| #2 | Q2 | 2 | #2 | IN3 | G11 |
| #2 | Q3 | 3 | #2 | IN4 | G12 |
| #2 | Q4 | 4 | #2 | IN5 | G13 |
| #2 | Q5 | 5 | #2 | IN6 | G14 |
| #2 | Q6 | 6 | #2 | IN7 | G15 |
| #2 | Q7 | 7 | #2 | IN8 | G16 |
| #3 | Q0 | 15 | #3 | IN1 | G17 |
| #3 | Q1 | 1 | #3 | IN2 | G18 |
| #3 | Q2 | 2 | #3 | IN3 | G19 |
| #3 | Q3 | 3 | #3 | IN4 | G20 |
| #3 | Q4 | 4 | #3 | IN5 | G21 |
| #3 | Q5 | 5 | #3 | IN6 | G22 |
| #3 | Q6 | 6 | #3 | IN7 | G23 |
| #3 | Q7 | 7 | #3 | IN8 | G24 |
| #4 | Q0 | 15 | #4 | IN1 | G25 |
| #4 | Q1 | 1 | #4 | IN2 | G26 |
| #4 | Q2 | 2 | #4 | IN3 | G27 |
| #4 | Q3 | 3 | #4 | IN4 | G28 |
| #4 | Q4 | 4 | #4 | IN5 | G29 |
| #4 | Q5 | 5 | #4 | IN6 | G30 |
| #4 | Q6 | 6 | #4 | IN7 | G31 |
| #4 | Q7 | 7 | #4 | IN8 | G32 |
| #5 | Q0 | 15 | #5 | IN1 | G33 |
| #5 | Q1 | 1 | #5 | IN2 | G34 |
| #5 | Q2 | 2 | #5 | IN3 | G35 |
| #5 | Q3 | 3 | #5 | IN4 | G36 |
| #5 | Q4 | 4 | #5 | IN5 | G37 |
| #5 | Q5 | 5 | #5 | IN6 | G38 |
| #5 | Q6 | 6 | #5 | IN7 | G39 |
| #5 | Q7 | 7 | #5 | IN8 | G40 |
| #6 | Q0-Q7 | 15,1-7 | #6 | IN1-IN8 | NC |
| #7 | Q0-Q7 | 15,1-7 | #7 | IN1-IN8 | NC |

**Important :** le code utilise `MSBFIRST` pour que le bit correspondant à Q0 arrive réellement sur Q0 dans cette chaîne de 74HC595.

---

## 9. ULN2803A — brochage réel

Pour chaque ULN2803A :

| Pin | Nom | Connexion |
|---:|---|---|
| 1 | IN1 | sortie Q0 du 74HC595 |
| 2 | IN2 | sortie Q1 |
| 3 | IN3 | sortie Q2 |
| 4 | IN4 | sortie Q3 |
| 5 | IN5 | sortie Q4 |
| 6 | IN6 | sortie Q5 |
| 7 | IN7 | sortie Q6 |
| 8 | IN8 | sortie Q7 |
| 9 | GND | GND commun |
| 10 | COM | NC pour ces LED sans bobine |
| 11 | OUT8 | cathodes groupe correspondant |
| 12 | OUT7 | cathodes groupe correspondant |
| 13 | OUT6 | cathodes groupe correspondant |
| 14 | OUT5 | cathodes groupe correspondant |
| 15 | OUT4 | cathodes groupe correspondant |
| 16 | OUT3 | cathodes groupe correspondant |
| 17 | OUT2 | cathodes groupe correspondant |
| 18 | OUT1 | cathodes groupe correspondant |

Le ULN2803A ne fournit pas le +12 V : il **commute la masse** des groupes de LED.

---

## 10. Un groupe de 4 LED contour

Pour chaque groupe G01 à G40 :

| Départ | Va vers |
|---|---|
| +12 V | Résistance LED 1 |
| Résistance LED 1 | Anode LED 1 |
| +12 V | Résistance LED 2 |
| Résistance LED 2 | Anode LED 2 |
| +12 V | Résistance LED 3 |
| Résistance LED 3 | Anode LED 3 |
| +12 V | Résistance LED 4 |
| Résistance LED 4 | Anode LED 4 |
| Cathode LED 1 | même sortie OUT du ULN du groupe |
| Cathode LED 2 | même sortie OUT du ULN du groupe |
| Cathode LED 3 | même sortie OUT du ULN du groupe |
| Cathode LED 4 | même sortie OUT du ULN du groupe |

Chaque LED doit avoir **sa propre résistance**. La valeur doit être calculée selon la couleur et le Vf réel ; une valeur de départ typique à 12 V est **1 kΩ**, à vérifier par mesure de courant avant fonctionnement prolongé.

---

## 11. DS3231

| DS3231 | Arduino Nano |
|---|---|
| VCC | +5 V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

---

## 12. Buzzer

| Élément | Connexion |
|---|---|
| D6 | + buzzer passif |
| - buzzer | GND |

Le programme utilise `tone()` pour la mélodie de démarrage et les sons de modification.

---

## 13. Règle importante de validation

Avant la mise sous tension complète :

1. Régler le buck à 5,0 V sans Arduino.
2. Vérifier la polarité du 12 V.
3. Vérifier le GND commun.
4. Vérifier chaque pin du MAX7219.
5. Vérifier chaque chaîne Q7S → DS des 74HC595.
6. Vérifier chaque Q0-Q7 → IN1-IN8 du ULN.
7. Vérifier que les 40 groupes sont bien G01-G40.
8. Vérifier une résistance individuelle par LED.
9. Vérifier que les sorties non utilisées des 74HC595 #6 et #7 restent sans charge.

**Référence finale : 114 LED affichage + 160 LED contour = 274 LED.**
