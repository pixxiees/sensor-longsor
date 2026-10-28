#include "fuzzy_logic.h"
#include <Fuzzy.h>

Fuzzy *fuzzy = new Fuzzy();

// ==========================================
// Himpunan Fuzzy (Disesuaikan)
// ==========================================
// Curah hujan (Tip / mm)
FuzzySet *hKering  = new FuzzySet(0, 0, 0.5, 2);
FuzzySet *hGerimis = new FuzzySet(1, 3, 8, 12);
FuzzySet *hDeras   = new FuzzySet(10, 15, 50, 50);

// Soil moisture (%)
FuzzySet *tRendah = new FuzzySet(0, 0, 30, 40);
FuzzySet *tSedang = new FuzzySet(30, 45, 55, 70);
FuzzySet *tTinggi = new FuzzySet(60, 75, 100, 100);

// Pergerakan/Kemiringan (derajat)
FuzzySet *pAman    = new FuzzySet(0, 0, 2, 4);
FuzzySet *pWaspada = new FuzzySet(3, 5, 8, 12);
FuzzySet *pBahaya  = new FuzzySet(10, 18, 90, 90);

// Output status (0-100) - Rentang sAwas diperlebar ke kiri
FuzzySet *sAman    = new FuzzySet(0, 0, 15, 35);
FuzzySet *sWaspada = new FuzzySet(25, 45, 55, 65);
FuzzySet *sAwas    = new FuzzySet(50, 70, 100, 100); // Batas sAwas dimulai dari 50

void tambahAturan(int id, FuzzySet *hujan, FuzzySet *tanah, FuzzySet *gerak, FuzzySet *status) {
    FuzzyRuleAntecedent *ifHujanDanTanah = new FuzzyRuleAntecedent();
    ifHujanDanTanah->joinWithAND(hujan, tanah);
    
    FuzzyRuleAntecedent *ifKondisi = new FuzzyRuleAntecedent();
    ifKondisi->joinWithAND(ifHujanDanTanah, gerak);
    
    FuzzyRuleConsequent *thenStatus = new FuzzyRuleConsequent();
    thenStatus->addOutput(status);
    
    FuzzyRule *aturan = new FuzzyRule(id, ifKondisi, thenStatus);
    fuzzy->addFuzzyRule(aturan);
}

void setupFuzzy() {
    FuzzyInput *hujan = new FuzzyInput(1);
    hujan->addFuzzySet(hKering); hujan->addFuzzySet(hGerimis); hujan->addFuzzySet(hDeras);
    fuzzy->addFuzzyInput(hujan);

    FuzzyInput *kelembapan = new FuzzyInput(2);
    kelembapan->addFuzzySet(tRendah); kelembapan->addFuzzySet(tSedang); kelembapan->addFuzzySet(tTinggi);
    fuzzy->addFuzzyInput(kelembapan);

    FuzzyInput *pergerakan = new FuzzyInput(3);
    pergerakan->addFuzzySet(pAman); pergerakan->addFuzzySet(pWaspada); pergerakan->addFuzzySet(pBahaya);
    fuzzy->addFuzzyInput(pergerakan);

    FuzzyOutput *status = new FuzzyOutput(1);
    status->addFuzzySet(sAman); status->addFuzzySet(sWaspada); status->addFuzzySet(sAwas);
    fuzzy->addFuzzyOutput(status);

    // 27 Basis Aturan Sesuai Proposal Slide 8
    tambahAturan(1, hKering, tRendah, pAman, sAman);
    tambahAturan(2, hKering, tRendah, pWaspada, sWaspada);
    tambahAturan(3, hKering, tRendah, pBahaya, sAwas);
    tambahAturan(4, hKering, tSedang, pAman, sAman);
    tambahAturan(5, hKering, tSedang, pWaspada, sWaspada);
    tambahAturan(6, hKering, tSedang, pBahaya, sAwas);
    tambahAturan(7, hKering, tTinggi, pAman, sAman);
    tambahAturan(8, hKering, tTinggi, pWaspada, sWaspada);
    tambahAturan(9, hKering, tTinggi, pBahaya, sAwas);

    tambahAturan(10, hGerimis, tRendah, pAman, sAman);
    tambahAturan(11, hGerimis, tRendah, pWaspada, sWaspada);
    tambahAturan(12, hGerimis, tRendah, pBahaya, sAwas);
    tambahAturan(13, hGerimis, tSedang, pAman, sAman);
    tambahAturan(14, hGerimis, tSedang, pWaspada, sWaspada);
    tambahAturan(15, hGerimis, tSedang, pBahaya, sAwas);
    tambahAturan(16, hGerimis, tTinggi, pAman, sWaspada);
    tambahAturan(17, hGerimis, tTinggi, pWaspada, sAwas);
    tambahAturan(18, hGerimis, tTinggi, pBahaya, sAwas);

    tambahAturan(19, hDeras, tRendah, pAman, sAman);
    tambahAturan(20, hDeras, tRendah, pWaspada, sWaspada);
    tambahAturan(21, hDeras, tRendah, pBahaya, sAwas);
    tambahAturan(22, hDeras, tSedang, pAman, sWaspada);
    tambahAturan(23, hDeras, tSedang, pWaspada, sAwas);
    tambahAturan(24, hDeras, tSedang, pBahaya, sAwas);
    tambahAturan(25, hDeras, tTinggi, pAman, sWaspada);
    tambahAturan(26, hDeras, tTinggi, pWaspada, sAwas);
    tambahAturan(27, hDeras, tTinggi, pBahaya, sAwas);
}

int hitungFuzzy(float dataHujan, float dataKelembapan, float dataPergerakan) {
    // Gunakan nilai absolut pergerakan sudut
    float absPergerakan = abs(dataPergerakan);

    fuzzy->setInput(1, dataHujan);
    fuzzy->setInput(2, dataKelembapan);
    fuzzy->setInput(3, absPergerakan);
    fuzzy->fuzzify();

    float hasilPeringatan = fuzzy->defuzzify(1);
    
    Serial.printf("\r\n[FUZZY] Input -> Hujan: %.1f, Soil: %.1f %%, Tilt: %.2f deg\r\n", dataHujan, dataKelembapan, absPergerakan);
    Serial.printf("[FUZZY] Nilai Defuzzifikasi: %.2f\r\n", hasilPeringatan);

    // KOREKSI THRESHOLD DEFUZZIFIKASI
    if (hasilPeringatan >= 50.0f) {  // Turunkan dari 60.0f ke 50.0f agar masuk ke BAHAYA (Rule 3, 6, 9)
        Serial.println("[FUZZY] KEPUTUSAN: BAHAYA (Lampu / Buzzer ON) 🔴");
        return 3; // Status 3: BAHAYA
    } else if (hasilPeringatan >= 25.0f) {
        Serial.println("[FUZZY] KEPUTUSAN: WASPADA 🟡");
        return 2; // Status 2: WASPADA
    } else {
        Serial.println("[FUZZY] KEPUTUSAN: AMAN 🟢");
        return 1; // Status 1: AMAN
    }
}