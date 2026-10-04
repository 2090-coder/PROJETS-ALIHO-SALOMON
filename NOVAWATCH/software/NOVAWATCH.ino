#include <Wire.h>

// ============================================================
// NOVAWATCH
// Arduino Nano + HT16K33 + DS3231 + 7x74HC595 + 7xULN2803A
// Alimentation principale : 12 V DC
// Affichage : 4 chiffres 7 segments + 2 LED pour les deux points
// Contour : 40 groupes de 4 LED = 160 LED
// ============================================================

// -------------------- Broches Arduino -----------------------

const byte BROCHE_BOUTON_ALIMENTATION = 2;
const byte BROCHE_BOUTON_MODE = 3;
const byte BROCHE_BOUTON_PLUS = 4;
const byte BROCHE_BOUTON_MOINS = 5;
const byte BROCHE_AVERTISSEUR = 6;
const byte BROCHE_DEUX_POINTS = 7;

const byte BROCHE_595_DONNEES = 8;
const byte BROCHE_595_VERROU = 9;
const byte BROCHE_HORLOGE = 13;

// A4 = SDA et A5 = SCL pour le bus I2C.

// -------------------- Adresses I2C ---------------------------

const byte ADRESSE_I2C_DS3231 = 0x68;
const byte ADRESSE_I2C_HT16K33 = 0x70;

// -------------------- Commandes HT16K33 ---------------------

const byte HT16K33_COMMANDE_SYSTEME = 0x20;
const byte HT16K33_OSCILLATEUR_ACTIF = 0x01;
const byte HT16K33_COMMANDE_AFFICHAGE = 0x80;
const byte HT16K33_AFFICHAGE_ACTIF = 0x01;
const byte HT16K33_COMMANDE_LUMINOSITE = 0xE0;
const byte HT16K33_LUMINOSITE = 8;

// Le HT16K33 possede ROW0 a ROW15 et COM0 a COM7.
// Pour cet affichage, ROW0 a ROW7 sont utilises pour A a G et DP.
// COM0 a COM3 sont utilises pour les quatre chiffres.
// Sur un module repere A0..A15 et C0..C7, verifier la serigraphie
// avant de connecter le panneau.

const byte HT16K33_ROW0_SEGMENT_A = 0;
const byte HT16K33_ROW1_SEGMENT_B = 1;
const byte HT16K33_ROW2_SEGMENT_C = 2;
const byte HT16K33_ROW3_SEGMENT_D = 3;
const byte HT16K33_ROW4_SEGMENT_E = 4;
const byte HT16K33_ROW5_SEGMENT_F = 5;
const byte HT16K33_ROW6_SEGMENT_G = 6;
const byte HT16K33_ROW7_POINT_DECIMAL = 7;

const byte HT16K33_COM0_CHIFFRE1 = 0;
const byte HT16K33_COM1_CHIFFRE2 = 1;
const byte HT16K33_COM2_CHIFFRE3 = 2;
const byte HT16K33_COM3_CHIFFRE4 = 3;

// RAM du HT16K33 :
// COM0 = 0x00/0x01, COM1 = 0x02/0x03,
// COM2 = 0x04/0x05, COM3 = 0x06/0x07.
// Le premier octet contient ROW0 a ROW7.

const byte HT16K33_RAM_COM0 = 0x00;
const byte HT16K33_RAM_COM1 = 0x02;
const byte HT16K33_RAM_COM2 = 0x04;
const byte HT16K33_RAM_COM3 = 0x06;

const byte HT16K33_SEGMENT_A = 0x01;
const byte HT16K33_SEGMENT_B = 0x02;
const byte HT16K33_SEGMENT_C = 0x04;
const byte HT16K33_SEGMENT_D = 0x08;
const byte HT16K33_SEGMENT_E = 0x10;
const byte HT16K33_SEGMENT_F = 0x20;
const byte HT16K33_SEGMENT_G = 0x40;
const byte HT16K33_POINT_DECIMAL = 0x80;

const byte HT16K33_MASQUE_CHIFFRES[10] = {
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_E | HT16K33_SEGMENT_F,
  HT16K33_SEGMENT_B | HT16K33_SEGMENT_C,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_D | HT16K33_SEGMENT_E | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_B | HT16K33_SEGMENT_C | HT16K33_SEGMENT_F | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_F | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_E | HT16K33_SEGMENT_F | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_C,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_E | HT16K33_SEGMENT_F | HT16K33_SEGMENT_G,
  HT16K33_SEGMENT_A | HT16K33_SEGMENT_B | HT16K33_SEGMENT_C | HT16K33_SEGMENT_D | HT16K33_SEGMENT_F | HT16K33_SEGMENT_G
};

// -------------------- Contour --------------------------------

const byte NOMBRE_595 = 7;
const byte NOMBRE_GROUPES_CONTOUR = 40;
byte donnees_595[NOMBRE_595] = {0};
byte groupe_contour = 0;
unsigned long derniere_mise_a_jour_contour = 0;
const unsigned long INTERVALLE_CONTOUR = 100;

// -------------------- Horloge --------------------------------

byte heure_actuelle = 0;
byte minute_actuelle = 0;
byte seconde_actuelle = 0;
unsigned long derniere_lecture_ds3231 = 0;
const unsigned long INTERVALLE_LECTURE_DS3231 = 500;

// -------------------- Etats -----------------------------------

bool montre_active = false;
bool demarrage_actif = false;
unsigned long debut_demarrage = 0;
unsigned long derniere_image_demarrage = 0;
byte image_demarrage = 0;
const unsigned long DUREE_DEMARRAGE = 2300;
const unsigned long INTERVALLE_IMAGE_DEMARRAGE = 120;

enum ChampReglage {
  REGLAGE_HEURE,
  REGLAGE_MINUTE
};

bool mode_reglage = false;
ChampReglage champ_reglage = REGLAGE_HEURE;
byte heure_reglage = 0;
byte minute_reglage = 0;
bool affichage_reglage_visible = true;
unsigned long dernier_clignotement = 0;
const unsigned long INTERVALLE_CLIGNOTEMENT = 350;

// -------------------- Boutons --------------------------------

struct Bouton {
  byte broche;
  bool lecture_brute;
  bool etat_stable;
  unsigned long dernier_changement;
};

Bouton bouton_alimentation = {BROCHE_BOUTON_ALIMENTATION, HIGH, HIGH, 0};
Bouton bouton_mode = {BROCHE_BOUTON_MODE, HIGH, HIGH, 0};
Bouton bouton_plus = {BROCHE_BOUTON_PLUS, HIGH, HIGH, 0};
Bouton bouton_moins = {BROCHE_BOUTON_MOINS, HIGH, HIGH, 0};

const unsigned long DELAI_ANTI_REBOND = 35;
const unsigned long FENETRE_CLICS_MODE = 1000;
byte nombre_clics_mode = 0;
unsigned long fin_fenetre_mode = 0;

// -------------------- Avertisseur ----------------------------

struct NoteMusicale {
  unsigned int frequence;
  unsigned int duree;
};

const NoteMusicale MELODIE_DEMARRAGE[] = {
  {523, 120}, {659, 120}, {784, 120}, {1047, 220},
  {784, 120}, {659, 120}, {523, 260}
};

const byte NOMBRE_NOTES_DEMARRAGE =
  sizeof(MELODIE_DEMARRAGE) / sizeof(MELODIE_DEMARRAGE[0]);

byte index_note = 0;
unsigned long prochaine_note = 0;

// -------------------- Prototypes -----------------------------

bool ds3231_disponible();
bool ht16k33_disponible();

// ============================================================
// Fonctions generales
// ============================================================

byte convertir_bcd_vers_decimal(byte valeur) {
  return ((valeur >> 4) * 10) + (valeur & 0x0F);
}

byte convertir_decimal_vers_bcd(byte valeur) {
  return ((valeur / 10) << 4) | (valeur % 10);
}

// ============================================================
// HT16K33
// ============================================================

void ht16k33_envoyer_commande(byte commande) {
  Wire.beginTransmission(ADRESSE_I2C_HT16K33);
  Wire.write(commande);
  Wire.endTransmission();
}

bool ht16k33_disponible() {
  Wire.beginTransmission(ADRESSE_I2C_HT16K33);
  return Wire.endTransmission() == 0;
}

void ht16k33_effacer_ram() {
  Wire.beginTransmission(ADRESSE_I2C_HT16K33);
  Wire.write(0x00);

  for (byte index_octet = 0; index_octet < 16; index_octet++) {
    Wire.write(0x00);
  }

  Wire.endTransmission();
}

void ht16k33_initialiser() {
  ht16k33_envoyer_commande(
    HT16K33_COMMANDE_SYSTEME | HT16K33_OSCILLATEUR_ACTIF
  );

  ht16k33_effacer_ram();

  ht16k33_envoyer_commande(
    HT16K33_COMMANDE_AFFICHAGE | HT16K33_AFFICHAGE_ACTIF
  );

  ht16k33_envoyer_commande(
    HT16K33_COMMANDE_LUMINOSITE | (HT16K33_LUMINOSITE & 0x0F)
  );
}

// Ecrit les bits ROW0 a ROW7 pour un COM.
// Le deuxieme octet ROW8 a ROW15 reste a zero.
void ht16k33_ecrire_chiffre(byte index_chiffre, byte segments) {
  if (index_chiffre > 3) {
    return;
  }

  const byte adresses_ram[4] = {
    HT16K33_RAM_COM0,
    HT16K33_RAM_COM1,
    HT16K33_RAM_COM2,
    HT16K33_RAM_COM3
  };

  byte adresse_ram = adresses_ram[index_chiffre];

  Wire.beginTransmission(ADRESSE_I2C_HT16K33);
  Wire.write(adresse_ram);
  Wire.write(segments);
  Wire.write(0x00);
  Wire.endTransmission();
}

void ht16k33_effacer_affichage() {
  for (byte index_chiffre = 0; index_chiffre < 4; index_chiffre++) {
    ht16k33_ecrire_chiffre(index_chiffre, 0x00);
  }
}

void ht16k33_afficher_heure(byte heure, byte minute) {
  if (heure > 23 || minute > 59) {
    return;
  }

  ht16k33_ecrire_chiffre(0, HT16K33_MASQUE_CHIFFRES[heure / 10]);
  ht16k33_ecrire_chiffre(1, HT16K33_MASQUE_CHIFFRES[heure % 10]);
  ht16k33_ecrire_chiffre(2, HT16K33_MASQUE_CHIFFRES[minute / 10]);
  ht16k33_ecrire_chiffre(3, HT16K33_MASQUE_CHIFFRES[minute % 10]);
}

void ht16k33_afficher_reglage() {
  byte dizaine_heure = heure_reglage / 10;
  byte unite_heure = heure_reglage % 10;
  byte dizaine_minute = minute_reglage / 10;
  byte unite_minute = minute_reglage % 10;

  if (champ_reglage == REGLAGE_HEURE && !affichage_reglage_visible) {
    ht16k33_ecrire_chiffre(0, 0x00);
    ht16k33_ecrire_chiffre(1, 0x00);
  } else {
    ht16k33_ecrire_chiffre(0, HT16K33_MASQUE_CHIFFRES[dizaine_heure]);
    ht16k33_ecrire_chiffre(1, HT16K33_MASQUE_CHIFFRES[unite_heure]);
  }

  if (champ_reglage == REGLAGE_MINUTE && !affichage_reglage_visible) {
    ht16k33_ecrire_chiffre(2, 0x00);
    ht16k33_ecrire_chiffre(3, 0x00);
  } else {
    ht16k33_ecrire_chiffre(2, HT16K33_MASQUE_CHIFFRES[dizaine_minute]);
    ht16k33_ecrire_chiffre(3, HT16K33_MASQUE_CHIFFRES[unite_minute]);
  }
}

void regler_deux_points(bool actif) {
  digitalWrite(BROCHE_DEUX_POINTS, actif ? HIGH : LOW);
}

// ============================================================
// DS3231
// ============================================================

bool ds3231_disponible() {
  Wire.beginTransmission(ADRESSE_I2C_DS3231);
  return Wire.endTransmission() == 0;
}

bool ds3231_lire_heure() {
  Wire.beginTransmission(ADRESSE_I2C_DS3231);
  Wire.write(0x00);

  if (Wire.endTransmission() != 0) {
    return false;
  }

  if (Wire.requestFrom(ADRESSE_I2C_DS3231, (byte)3) != 3) {
    return false;
  }

  seconde_actuelle = convertir_bcd_vers_decimal(Wire.read() & 0x7F);
  minute_actuelle = convertir_bcd_vers_decimal(Wire.read() & 0x7F);
  heure_actuelle = convertir_bcd_vers_decimal(Wire.read() & 0x3F);

  if (heure_actuelle > 23 || minute_actuelle > 59 || seconde_actuelle > 59) {
    return false;
  }

  return true;
}

bool ds3231_ecrire_heure(byte heure, byte minute, byte seconde) {
  if (heure > 23 || minute > 59 || seconde > 59) {
    return false;
  }

  Wire.beginTransmission(ADRESSE_I2C_DS3231);
  Wire.write(0x00);
  Wire.write(convertir_decimal_vers_bcd(seconde));
  Wire.write(convertir_decimal_vers_bcd(minute));
  Wire.write(convertir_decimal_vers_bcd(heure));

  return Wire.endTransmission() == 0;
}

// ============================================================
// 74HC595 + ULN2803A + contour
// ============================================================

void effacer_donnees_595() {
  for (byte index_595 = 0; index_595 < NOMBRE_595; index_595++) {
    donnees_595[index_595] = 0x00;
  }
}

void regler_groupe_contour(byte groupe, bool actif) {
  if (groupe >= NOMBRE_GROUPES_CONTOUR) {
    return;
  }

  byte index_595 = groupe / 8;
  byte bit_sortie = groupe % 8;

  if (actif) {
    donnees_595[index_595] |= (byte)(1 << bit_sortie);
  } else {
    donnees_595[index_595] &= (byte)~(1 << bit_sortie);
  }
}

void ecrire_donnees_595() {
  digitalWrite(BROCHE_595_VERROU, LOW);

  // Le dernier CI doit etre envoye en premier dans cette chaine.
  for (int index_595 = NOMBRE_595 - 1; index_595 >= 0; index_595--) {
    shiftOut(
      BROCHE_595_DONNEES,
      BROCHE_HORLOGE,
      MSBFIRST,
      donnees_595[index_595]
    );
  }

  digitalWrite(BROCHE_595_VERROU, HIGH);
}

void initialiser_contour() {
  pinMode(BROCHE_595_DONNEES, OUTPUT);
  pinMode(BROCHE_595_VERROU, OUTPUT);
  pinMode(BROCHE_HORLOGE, OUTPUT);

  effacer_donnees_595();
  ecrire_donnees_595();
}

void mettre_a_jour_contour() {
  if (!montre_active) {
    return;
  }

  if (millis() - derniere_mise_a_jour_contour < INTERVALLE_CONTOUR) {
    return;
  }

  derniere_mise_a_jour_contour = millis();

  effacer_donnees_595();
  regler_groupe_contour(groupe_contour, true);
  ecrire_donnees_595();

  groupe_contour++;

  if (groupe_contour >= NOMBRE_GROUPES_CONTOUR) {
    groupe_contour = 0;
  }
}

// ============================================================
// Avertisseur et melodie
// ============================================================

void bip_action() {
  tone(BROCHE_AVERTISSEUR, 880, 60);
}

void bip_chiffre(byte chiffre) {
  const unsigned int frequences[10] = {
    262, 294, 330, 349, 392, 440, 494, 523, 587, 659
  };

  tone(BROCHE_AVERTISSEUR, frequences[chiffre % 10], 75);
}

void demarrer_melodie() {
  index_note = 0;
  prochaine_note = 0;
}

void mettre_a_jour_melodie() {
  if (!demarrage_actif) {
    return;
  }

  unsigned long maintenant = millis();

  if (maintenant < prochaine_note) {
    return;
  }

  if (index_note >= NOMBRE_NOTES_DEMARRAGE) {
    noTone(BROCHE_AVERTISSEUR);
    prochaine_note = maintenant + 100000UL;
    return;
  }

  tone(
    BROCHE_AVERTISSEUR,
    MELODIE_DEMARRAGE[index_note].frequence,
    MELODIE_DEMARRAGE[index_note].duree - 10
  );

  prochaine_note = maintenant + MELODIE_DEMARRAGE[index_note].duree;
  index_note++;
}

// ============================================================
// Animation de demarrage
// ============================================================

void ht16k33_afficher_animation(byte image) {
  byte segments = 0x00;

  switch (image % 8) {
    case 0: segments = HT16K33_SEGMENT_A; break;
    case 1: segments = HT16K33_SEGMENT_B; break;
    case 2: segments = HT16K33_SEGMENT_C; break;
    case 3: segments = HT16K33_SEGMENT_D; break;
    case 4: segments = HT16K33_SEGMENT_E; break;
    case 5: segments = HT16K33_SEGMENT_F; break;
    case 6: segments = HT16K33_SEGMENT_G; break;
    case 7:
      segments =
        HT16K33_SEGMENT_A |
        HT16K33_SEGMENT_B |
        HT16K33_SEGMENT_C |
        HT16K33_SEGMENT_D |
        HT16K33_SEGMENT_E |
        HT16K33_SEGMENT_F |
        HT16K33_SEGMENT_G;
      break;
  }

  for (byte index_chiffre = 0; index_chiffre < 4; index_chiffre++) {
    ht16k33_ecrire_chiffre(index_chiffre, segments);
  }

  regler_deux_points((image % 2) == 0);
}

void demarrer_montre() {
  montre_active = true;
  mode_reglage = false;
  demarrage_actif = true;

  debut_demarrage = millis();
  derniere_image_demarrage = 0;
  image_demarrage = 0;
  groupe_contour = 0;

  demarrer_melodie();

  ht16k33_effacer_affichage();
  effacer_donnees_595();
  ecrire_donnees_595();
}

void arreter_montre() {
  montre_active = false;
  demarrage_actif = false;
  mode_reglage = false;
  nombre_clics_mode = 0;

  noTone(BROCHE_AVERTISSEUR);

  ht16k33_effacer_affichage();
  regler_deux_points(false);

  effacer_donnees_595();
  ecrire_donnees_595();
}

void mettre_a_jour_demarrage() {
  if (!demarrage_actif) {
    return;
  }

  unsigned long maintenant = millis();

  if (maintenant - derniere_image_demarrage >= INTERVALLE_IMAGE_DEMARRAGE) {
    derniere_image_demarrage = maintenant;
    ht16k33_afficher_animation(image_demarrage);
    image_demarrage++;
  }

  if (maintenant - debut_demarrage >= DUREE_DEMARRAGE) {
    demarrage_actif = false;
    noTone(BROCHE_AVERTISSEUR);

    if (!ds3231_lire_heure()) {
      heure_actuelle = 0;
      minute_actuelle = 0;
      seconde_actuelle = 0;
    }

    ht16k33_afficher_heure(heure_actuelle, minute_actuelle);
    regler_deux_points(true);
  }
}

// ============================================================
// Boutons
// ============================================================

bool bouton_presse(Bouton &bouton) {
  bool lecture = digitalRead(bouton.broche);

  if (lecture != bouton.lecture_brute) {
    bouton.lecture_brute = lecture;
    bouton.dernier_changement = millis();
  }

  if (
    millis() - bouton.dernier_changement >= DELAI_ANTI_REBOND &&
    lecture != bouton.etat_stable
  ) {
    bouton.etat_stable = lecture;

    if (bouton.etat_stable == LOW) {
      return true;
    }
  }

  return false;
}

void reinitialiser_horloge() {
  if (ds3231_ecrire_heure(0, 0, 0)) {
    heure_actuelle = 0;
    minute_actuelle = 0;
    seconde_actuelle = 0;
  }

  bip_action();
  ht16k33_afficher_heure(heure_actuelle, minute_actuelle);
}

void entrer_mode_reglage() {
  if (!ds3231_lire_heure()) {
    return;
  }

  heure_reglage = heure_actuelle;
  minute_reglage = minute_actuelle;
  champ_reglage = REGLAGE_HEURE;
  affichage_reglage_visible = true;
  mode_reglage = true;
  dernier_clignotement = millis();

  ht16k33_afficher_reglage();
  bip_action();
}

void valider_mode_reglage() {
  if (ds3231_ecrire_heure(heure_reglage, minute_reglage, 0)) {
    heure_actuelle = heure_reglage;
    minute_actuelle = minute_reglage;
    seconde_actuelle = 0;
  }

  mode_reglage = false;
  affichage_reglage_visible = true;

  ht16k33_afficher_heure(heure_actuelle, minute_actuelle);
  regler_deux_points(true);
  bip_action();
}

void traiter_clics_mode() {
  if (nombre_clics_mode == 0) {
    return;
  }

  if (millis() < fin_fenetre_mode) {
    return;
  }

  if (!mode_reglage) {
    if (nombre_clics_mode == 1) {
      reinitialiser_horloge();
    } else if (nombre_clics_mode == 2) {
      entrer_mode_reglage();
    }
  } else {
    if (nombre_clics_mode == 1) {
      champ_reglage =
        (champ_reglage == REGLAGE_HEURE)
        ? REGLAGE_MINUTE
        : REGLAGE_HEURE;

      affichage_reglage_visible = true;
      dernier_clignotement = millis();

      ht16k33_afficher_reglage();
      bip_action();
    } else if (nombre_clics_mode == 3) {
      valider_mode_reglage();
    }
  }

  nombre_clics_mode = 0;
}

void gerer_boutons() {
  if (bouton_presse(bouton_alimentation)) {
    if (montre_active) {
      arreter_montre();
    } else {
      demarrer_montre();
    }
  }

  if (!montre_active || demarrage_actif) {
    return;
  }

  if (bouton_presse(bouton_mode)) {
    nombre_clics_mode++;

    if (nombre_clics_mode > 3) {
      nombre_clics_mode = 3;
    }

    fin_fenetre_mode = millis() + FENETRE_CLICS_MODE;
  }

  if (mode_reglage) {
    if (bouton_presse(bouton_plus)) {
      if (champ_reglage == REGLAGE_HEURE) {
        heure_reglage = (heure_reglage + 1) % 24;
        bip_chiffre(heure_reglage % 10);
      } else {
        minute_reglage = (minute_reglage + 1) % 60;
        bip_chiffre(minute_reglage % 10);
      }

      affichage_reglage_visible = true;
      dernier_clignotement = millis();
      ht16k33_afficher_reglage();
    }

    if (bouton_presse(bouton_moins)) {
      if (champ_reglage == REGLAGE_HEURE) {
        heure_reglage = (heure_reglage == 0) ? 23 : heure_reglage - 1;
        bip_chiffre(heure_reglage % 10);
      } else {
        minute_reglage = (minute_reglage == 0) ? 59 : minute_reglage - 1;
        bip_chiffre(minute_reglage % 10);
      }

      affichage_reglage_visible = true;
      dernier_clignotement = millis();
      ht16k33_afficher_reglage();
    }
  }

  traiter_clics_mode();
}

void mettre_a_jour_clignotement_reglage() {
  if (!mode_reglage) {
    return;
  }

  if (millis() - dernier_clignotement >= INTERVALLE_CLIGNOTEMENT) {
    dernier_clignotement = millis();
    affichage_reglage_visible = !affichage_reglage_visible;
    ht16k33_afficher_reglage();
  }
}

// ============================================================
// Verification du bus I2C
// ============================================================

void verifier_bus_i2c() {
  byte appareils_trouves = 0;

  for (byte adresse = 1; adresse < 127; adresse++) {
    Wire.beginTransmission(adresse);

    if (Wire.endTransmission() == 0) {
      appareils_trouves++;
    }
  }

  Serial.print("Appareils I2C trouves : ");
  Serial.println(appareils_trouves);

  Serial.print("DS3231 : ");
  Serial.println(ds3231_disponible() ? "OK" : "ABSENT");

  Serial.print("HT16K33 : ");
  Serial.println(ht16k33_disponible() ? "OK" : "ABSENT");
}

// ============================================================
// Configuration et boucle principale
// ============================================================

void setup() {
  pinMode(BROCHE_BOUTON_ALIMENTATION, INPUT_PULLUP);
  pinMode(BROCHE_BOUTON_MODE, INPUT_PULLUP);
  pinMode(BROCHE_BOUTON_PLUS, INPUT_PULLUP);
  pinMode(BROCHE_BOUTON_MOINS, INPUT_PULLUP);

  pinMode(BROCHE_AVERTISSEUR, OUTPUT);
  pinMode(BROCHE_DEUX_POINTS, OUTPUT);

  digitalWrite(BROCHE_DEUX_POINTS, LOW);

  Serial.begin(115200);
  Wire.begin();

  initialiser_contour();

  if (ht16k33_disponible()) {
    ht16k33_initialiser();
  }

  ds3231_lire_heure();
  verifier_bus_i2c();

  montre_active = false;
  demarrage_actif = false;
  mode_reglage = false;
}

void loop() {
  gerer_boutons();

  if (!montre_active) {
    return;
  }

  mettre_a_jour_melodie();
  mettre_a_jour_demarrage();
  mettre_a_jour_contour();

  if (demarrage_actif) {
    return;
  }

  if (
    !mode_reglage &&
    millis() - derniere_lecture_ds3231 >= INTERVALLE_LECTURE_DS3231
  ) {
    derniere_lecture_ds3231 = millis();

    if (ds3231_lire_heure()) {
      ht16k33_afficher_heure(heure_actuelle, minute_actuelle);
    }
  }

  mettre_a_jour_clignotement_reglage();
}
