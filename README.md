Rail Maintenance Simulator
Un joc de simulare și strategie 3D dezvoltat în C++ folosind OpenGL. Jucătorul preia rolul unui mecanic care trebuie să mențină infrastructura feroviară funcțională în timp ce trenurile circulă autonom între stații.

Mecanici de Joc
- Sistem de Avarii Aleatorii: Segmentele de șină se deteriorează în timp (probabilitate random). Avariile sunt vizibile prin deformare geometrică și efecte de culoare/flicker.
- Reparații cu Drezina: Jucătorul controlează o drezină animată (WASD) și trebuie să ajungă la segmentele defecte pentru a le repara (tasta F).
- Trenuri Autonome: Locomotivele parcurg traseul și opresc automat în fața șinelor avariate. Acestea staționează în cele minim 3 stații înainte de a primi o destinație nouă.
- Condiții de Game Over: Jocul se termină dacă un tren așteaptă prea mult (peste 30s) sau dacă gradul de avariere al traseului depășește 50%.

Detalii Tehnice
- Randare 3D: Implementarea unui traseu complex cu porțiuni la suprafață și suspendate.
- Control Cameră: Perspectivă Third-Person (TPS) care urmărește dinamic mișcarea drezinei.
- Animații: Mișcare de balans pentru mânerul drezinei și transformări geometrice pentru segmentele avariate.
- Interfață (UI/Minimap): Mapă 2D care indică starea segmentelor (funcțional/avariat) și poziția jucătorului.