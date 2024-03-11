#define echoPin 12
#define trigPin 11
#define encoderA 2
#define encoderB 3
 
long duration = 0;
int distance = 0;
 
int leftEncoderTicks = 0;
int rightEncoderTicks = 0;
int leftSpeed = 0;
int rightSpeed = 0;
double DIAMETER = 0.065 * PI;
 
const int STOP_DISTANCE_THRESHOLD = 50; // Adjust the threshold as needed
const int MIN_DISTANCE_THRESHOLD = 100; // Minimalna odległość, aby zatrzymać robota
const int MEDIUM_DISTANCE_THRESHOLD = 150; // Średnia odległość, aby zwolnić do średniej prędkości
const int MAX_EXECUTION_TIME = 10000; // Maximum execution time in milliseconds
int delayValue = 1000;
 
int get_signal_son() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = duration * 0.034 / 2;
    return distance;
}
 
void copy(float *src, int len, float *dst) {
    memcpy(dst, src, sizeof(src[0]) * len);
}
 
void moveForward(int speed) {
    digitalWrite(A0, HIGH);
    digitalWrite(A3, HIGH);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
    updateMotorSpeeds(speed);
   
}
 
void setup() {
    Serial.begin(9600);
    pinMode(2, INPUT_PULLUP);
    pinMode(A0, OUTPUT);
    pinMode(A1, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(A2, OUTPUT);
    pinMode(A3, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(11, OUTPUT);
    pinMode(12, INPUT);
    pinMode(encoderA, INPUT);
    pinMode(encoderB, INPUT);
    attachInterrupt(digitalPinToInterrupt(encoderA), leftEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(encoderB), rightEncoderISR, CHANGE);
}
 
void loop() {
    //delay(500);  // Dodane opóźnienie w celu kontrolowania częstotliwości pętli sterowania
    unsigned long startMillis = millis();  // Początkowy czas wykonania zadania
 
    distance = get_signal_son();
    float odleglosc[6] = {50, 75, 100, 125, 150, 200};
    float big[6] = {0, 0.25, 0.35, 0.55, 0.85, 1};
    float med[6] = {0, 0.4, 0.9, 0.9, 0.4, 0};
    float sml[6] = {1, 0.85, 0.5, 0.25, 0, 0};
 
    int i = 0;
    for (; i < 6; i++) {
        if (odleglosc[i] > distance)
            break;
    }
 
    float bigX = big[i];
    float medX = med[i];
    float smlX = sml[i];
 
    float predkosc[6] = {0, 50, 100, 200, 255};
    float fast[6] = {0, 0.25, 0.5, 0.85, 1};
    float medFst[6] = {0, 0.6, 1, 0.6, 0};
    float slo[6] = {1, 0.85, 0.5, 0.25, 0};
    float fastX[6];
    copy(fast, 6, fastX);
    float medFstX[6];
    copy(medFst, 6, medFstX);
    float sloX[6];
    copy(slo, 6, sloX);
 
    for (int j = 0; j < 5; j++) {
        if (fastX[j] > bigX)
            fastX[j] = bigX;
    }
 
    for (int j = 0; j < 5; j++) {
        if (medFstX[j] > medX)
            medFstX[j] = medX;
    }
 
    for (int j = 0; j < 5; j++) {
        if (sloX[j] > smlX)
            sloX[j] = smlX;
    }
 
    float out[5] = {};
    for (int j = 0; j < 5; j++) {
        if (medFstX[j] > sloX[j]) {
            if (medFstX[j] > fastX[j])
                out[j] = medFstX[j];
            else
                out[j] = fastX[j];
        } else {
            if (sloX[j] > fastX[j])
                out[j] = sloX[j];
            else
                out[j] = fastX[j];
        }
    }

    float arg = 0;
    int sum = 0;
    for (int j = 0; j < 5; j++) {
        arg = arg + out[j] * predkosc[j];
        sum = sum + out[j];
    }

    if (sum != 0) {
        arg = arg / sum;
        Serial.print("arg ");
        Serial.println(arg);
        Serial.println(sum);
    } else {
        arg = arg / (arg + 1);
        Serial.println("sum is zero, default action taken");
    }

    // Kontrola prędkości w zależności od odległości
    int speed = static_cast<int>(arg);

    // Sprawdzenie warunku zatrzymania
    if (distance < STOP_DISTANCE_THRESHOLD) {
        stopMotors();
        Serial.println("Disqualified - Exceeded distance threshold");
    } else {
        // Kontrola prędkości w zależności od odległości
        if (distance < MIN_DISTANCE_THRESHOLD) {
            speed = 100;  // Zwolnij do prędkości 100, gdy odległość jest mniejsza niż MIN_DISTANCE_THRESHOLD
        } else if (distance < MEDIUM_DISTANCE_THRESHOLD) {
            // Zwolnij do średniej prędkości
            speed = map(distance, MIN_DISTANCE_THRESHOLD, MEDIUM_DISTANCE_THRESHOLD, 100, 150);  // Stopniowe zwalnianie do średniej prędkości
        }

        moveForward(speed);

        // Monitorowanie czasu wykonania zadania
        unsigned long currentMillis = millis();
        if (currentMillis - startMillis > MAX_EXECUTION_TIME) {
            Serial.println("Disqualified - Exceeded maximum execution time");
            stopMotors();
        }
    }
}
 
void leftEncoderISR() {
    leftEncoderTicks++;
}
 
void rightEncoderISR() {
    rightEncoderTicks++;
}
 
void updateMotorSpeeds(int speed) {
    int tickDifference = leftEncoderTicks - rightEncoderTicks;
 
    float a = 3;
    
    int leftSpeed = speed - a * tickDifference;
    int rightSpeed = speed + a * tickDifference;
 
    leftSpeed = constrain(leftSpeed, 0, 255);
    rightSpeed = constrain(rightSpeed, 0, 255);
 
    analogWrite(5, leftSpeed);
    analogWrite(6, rightSpeed);
}
 
void stopMotors() {
    digitalWrite(A0, LOW);
    digitalWrite(A1, LOW);
    digitalWrite(A2, LOW);
    digitalWrite(A3, LOW);
}
