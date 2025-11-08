# My Malloc

Dans ce projet je crée ma propre fonction malloc et son free. J'utlise une listen pour simuler la heap. La fonction `init()` permet d'initialiser la heap et préparer l'espace pour toute future allocation, donc elle doit être appelée avant l'utilisation de malloc ou free

## 1. Métadonnées (MD)

J'ai alloué six bytes de MD pour chaque bloc de mémoire libre ou alloué : quatre au début et deux à la fin. Au début du tableau (heap), deux bytes de MD sont alloués pour stocker l’indice du premier bloc libre dans le tableau afin de démarrer la liste chaînée des blocs vides.

Les MD à gauche contiennent deux informations :

- size : nombre d’octets disponibles
- next :
  - indice du prochain bloc libre si l’espace est libre
  - 0 si l’espace est occupé
  - 65.535 (uint16_t max) si le bloc est le dernier bloc libre

À droite il n’y a que l’information size.

### État initial (après l’initialisation)

```text
+----------------------+------------------------------+----------------+
| BEGIN BLOCK          | 63.992 bytes libres          | END BLOCK      |
| size : 63.992        | indice : 2 + 4(MD)           | size : 63.992  |
| next : 65.535        |                              |                |
+----------------------+------------------------------+----------------+
| FIRST FREE           |
| indice : 2           |
+----------------------+
```

## 2. Allocation de mémoire

Quand un bloc libre adéquat est trouvé pour accueillir les données-utilisateur, des blocs de MD sont placés au début et à la fin du nouvel espace occupé, mais aussi de l’espace vide restant. Si la taille du bloc restant est plus petite que 7 bytes, alors le bloc trouvé est entièrement alloué pour évite un fragment inutilisable. Le `next` du bloc alloué est mis à 0, les `next` des blocs vides précédent et suivant sont mise à jour et l'indice du premier bloc libre est au dbut de la heap est mis à jour si nécessaire.

### Exemple (après trois allocations)

```text
+-------------------+--------------------------------------+-------------------+
| BEGIN BLOCK       | 63.943 bytes libres                  | END BLOCK         |
| size : 63.943     | indice : 51 + 4(MD)                  | size : 63.943     |
| next : 65.535     |                                      |                   |
+-------------------+--------------------------------------+-------------------+
| BEGIN BLOCK       | 13 bytes de données-utilisateurs     | END BLOCK         |
| size : 13         | adress retournée : &MY_HEAP[36]      | size : 13         |
| next : 0          |                                      |                   |
+-------------------+--------------------------------------+-------------------+
| BEGIN BLOCK       | 17 bytes de données-utilisateurs     | END BLOCK         |
| size : 17         | adress retournée : &MY_HEAP[13]      | size : 17         |
| next : 0          |                                      |                   |
+-------------------+--------------------------------------+-------------------+
| BEGIN BLOCK       | 1 bytes de données-utilisateurs      | END BLOCK         |
| size : 1          | adress retournée : &MY_HEAP[6]       | size : 1          |
| next : 0          |                                      |                   |
+-------------------+--------------------------------------+-------------------+
| FIRST FREE        |
| indice : 51       |
+-------------------+
```

## 3. Libération de mémoire

La libération consiste en deux opérations :

1. Vérifier à gauche et à droite si les espaces adjacents sont libres pour les fusionner immédiatement (stratégie « eager »).
2. Mettre à jour les `next` des espaces vides autour afin d’insérer le nouveau bloc libre dans la liste chaînée des blocs vides.

## 4. Stratégie de placement

Objectif : compromis entre optimisation mémoire et vitesse. La recherche commence au début du tableau et saute de bloc vide en bloc vide via la liste chaînée.

Stratégies utilisées :

```text
+----------------------+--------------------------------------------------+
| Stratégie            | Condition / Description                          |
+----------------------+--------------------------------------------------+
| First perfect fit    | Taille exactement demandée (size + 6 bytes MD)   |
| Almost perfect fit   | Taille dans une marge de ~10%                    |
| Insignificant fit    | Laisserait un reste >= 10× la taille demandée    |
| Best fit             | Si aucune des précédentes ne s'applique          |
+----------------------+--------------------------------------------------+
```

Processus :

- Parcours séquentiel des blocs libres.
- Allocation immédiate si « perfect » ou « almost » ou « insignificant » fit.
- Sinon conservation de la meilleure adresse rencontrée (best fit) jusqu’à la fin.

### Résultats mesurés

```text
+-------------------------+-------+
| Indicateur              | Score |
+-------------------------+-------+
| Optimisation mémoire    | 77%   |
| Optimisation temps      | 75%   |
+-------------------------+-------+
```

## Comment run les tests

### Prérequis

Packages à installé:

- gcc and make avec Lunix
- Librairie CUnit  (headers et runtime)
- lcov: optionnel pour le coverage

Fedora:

```bash
sudo dnf install -y gcc make CUnit CUnit-devel lcov
```

Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install -y build-essential libcunit1 libcunit1-dev lcov
```

### Build et run tests

Dépuis la racine du projet (`my_malloc`):

```bash
make test
```

Ceci compile `malloc.c` et `test.c`, lie CUnit, et lance les tests.

### Exécuter le générateur de performances/résumé

```bash
make resume
```

Executer `test_resume.c` qui imprime les statistiques d'allocation/libre.

### Netoyer le dossier du projet

```bash
make clean
```

Efface les fichiers binaires (`test`, `resume`, `malloc.o`) et les fichiers coverage.

### Exécuter avec coverage en local (optionnel)

```bash
gcc -Wall -Werror --coverage -o malloc.o -c malloc.c
gcc -Wall -Werror --coverage -o test test.c malloc.o -lcunit -lm
./test
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info --ignore-errors unused
lcov --list coverage.info
```
