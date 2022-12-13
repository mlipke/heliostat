#include <math.h>

// ------------------------------------------------

struct Heliostat {
  int altitude;
  int azimuth;
};

struct Switch {
  int pin;
};

struct Motor {
  // position signals
  int pin_signal_A;
  int pin_signal_B;

  unsigned long signal_A;
  int signal_B;

  // direction switches
  int pin_switch_l;
  int pin_switch_r;

  int pos_switch_l;
  int pos_switch_r;
};

const unsigned int MOTOR_TURNS = 512;

const double M_DEG_RATIO = 360.0 / MOTOR_TURNS;

const double RPD = 0.01745329;

// ------------------------------------------------

// to initialize, align the mirror horizontally and
// the tilt axis with west-east
struct Heliostat heliostat = {
  .altitude = 180,
  .azimuth = 0
};

struct Switch set_pos = {
  .pin = 3
};

void set_pos_switch() {
  // set heliostat to current motor angles
  // TODO
}

struct Motor altitude = {
  2,
  3,
  0,
  0,
  12,
  13,
  0,
  0
};

void IRAM_ATTR altitude_signal_A() {
  altitude.signal_A++;
}

void IRAM_ATTR altitude_signal_B() {
  altitude.signal_B++;
}

struct Motor azimuth = {
  15,
  16,
  0,
  0,
  8,
  9,
  0,
  0
};

void IRAM_ATTR azimuth_signal_A() {
  azimuth.signal_A++;
}

void IRAM_ATTR azimuth_signal_B() {
  azimuth.signal_B++;
}


// ---------------------------------------

double rotation(Motor* m) {
  return (m->signal_A / 2 % MOTOR_TURNS) * M_DEG_RATIO;
}

void rot_l(Motor* m) {
  if (digitalRead(m->pin_switch_r) == HIGH) {
    digitalWrite(m->pin_switch_r, LOW);
  }

  digitalWrite(m->pin_switch_l, HIGH);
}

void rot_r(Motor* m) {
  if (digitalRead(m->pin_switch_l) == HIGH) {
    digitalWrite(m->pin_switch_l, LOW);
  }

  digitalWrite(m->pin_switch_r, HIGH);
}

void stop(Motor* m) {
  digitalWrite(m->pin_switch_r, LOW);
  digitalWrite(m->pin_switch_l, LOW);
}

void setup_motor(Motor* m) {
  pinMode(m->pin_signal_A, INPUT_PULLUP);
  pinMode(m->pin_signal_B, INPUT_PULLUP);

  pinMode(m->pin_switch_l, OUTPUT);
  pinMode(m->pin_switch_r, OUTPUT);
}

void setup() {
  attachInterrupt(set_pos.pin, set_pos_switch, FALLING);

  setup_motor(&altitude);
  setup_motor(&azimuth);

  attachInterrupt(altitude.pin_signal_A, altitude_signal_A, CHANGE);
  attachInterrupt(altitude.pin_signal_B, altitude_signal_B, CHANGE);

  attachInterrupt(azimuth.pin_signal_A, azimuth_signal_A, CHANGE);
  attachInterrupt(azimuth.pin_signal_B, azimuth_signal_B, CHANGE);

  Serial.begin(115200);
  Serial.println("Starting up!");
}

void print_current_position() {
  Serial.print("Azimuth: ");
  Serial.print(rotation(&azimuth));
  Serial.print(" (");
  Serial.print(azimuth.signal_A);
  Serial.print(")");
  Serial.println("");

  Serial.print("Altitude: ");
  Serial.print(rotation(&altitude));
  Serial.print(" (");
  Serial.print(altitude.signal_A);
  Serial.print(")");
  Serial.println("");
}

unsigned long simulator_steps = 0;

void simulate_turn_l(Motor* m) {
  if (simulator_steps % 2 == 0) {
    m->signal_A -= 1;
  }

  if (simulator_steps % 2 == 1) {
    m->signal_B -= 1;
  }
}

void simulate_turn_r(Motor* m) {
  if (simulator_steps % 2 == 0) {
    m->signal_A += 1;
  }

  if (simulator_steps % 2 == 1) {
    m->signal_B += 1;
  }
}

void simulator() {
  // if (simulator_steps < 800) {
  //   simulate_turn_r(&azimuth);
  // }

  // if (simulator_steps < 600) {
  //   simulate_turn_r(&altitude);
  // }

  if (digitalRead(altitude.pin_switch_r) == HIGH) {
    simulate_turn_r(&altitude);
  }

  if (digitalRead(altitude.pin_switch_l) == HIGH) {
    simulate_turn_l(&altitude);
  }

    if (digitalRead(azimuth.pin_switch_r) == HIGH) {
    simulate_turn_r(&azimuth);
  }

  if (digitalRead(azimuth.pin_switch_l) == HIGH) { 
    simulate_turn_l(&azimuth);
  }

  simulator_steps++;
}

void sunpos() {
  // TODO get date & time

  // TODO calculate days since J2000

  // TODO calculate current solar position
  double sun_az = 180.0;
  double sun_al = 35.0;

  // TODO check if sun is up

  // get current azimuth and altitude
  double az = rotation(&azimuth);
  double al = rotation(&altitude);

  // calculate desired azimuth and altitude
  double desired_az = (heliostat.azimuth - sun_az) / 2.0 + sun_az;
  double desired_al = (heliostat.altitude - sun_al) / 2.0 + sun_al; 

  // check which direction to move in
  

  // move mirror into position
  if (az < desired_az) {
    rot_r(&azimuth);
  } else {
    stop(&azimuth);
  }

  if (al < desired_al) {
    rot_l(&altitude);
  } else {
    stop(&altitude);
  }

  print_current_position();
}

void loop() {
  simulator();

  if (simulator_steps % 50 == 0) {
    sunpos();
  }

  delay(10);
}
