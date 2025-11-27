#include <iomanip>
#include <iostream>
#include <string>
using namespace std;

#define MAX_FILES 12
#define NUM_CLUSTERS 12
#define CLUSTER_SIZE 1024
#define FAT_LIBRE 0
#define FAT_OCUPADO -2
#define FAT_EOF -1

struct File {
  string fileName;
  int size;
  int start_cluster;
  bool isActive;

  File() : fileName(""), size(0), start_cluster(-1), isActive(false) {}
};

class FATSystem {
private:
  File raiz[MAX_FILES];
  int FAT_TABLE[NUM_CLUSTERS];

public:
  FATSystem() { InitializeFAT(); }

  void InitializeFAT() {
    for (int i = 0; i < NUM_CLUSTERS; i++) {
      FAT_TABLE[i] = FAT_LIBRE;
    }

    for (int j = 0; j < MAX_FILES; j++) {
      raiz[j].fileName = "";
      raiz[j].size = 0;
      raiz[j].start_cluster = -1;
      raiz[j].isActive = false;
    }

    cout << "Sistema FAT inicializado correctamente." << endl;
  }

  int FindFreeCluster() {
    for (int i = 0; i < NUM_CLUSTERS; i++) {
      if (FAT_TABLE[i] == FAT_LIBRE) {
        return i;
      }
    }
    return -1;
  }

  bool AllocateFile(string filename, int contenido) {
    int clusters_needed = (contenido + CLUSTER_SIZE - 1) / CLUSTER_SIZE;

    if (clusters_needed > NUM_CLUSTERS) {
      cout << "Error: El archivo es demasiado grande." << endl;
      return false;
    }
    int directorio_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
      if (!raiz[i].isActive) {
        directorio_index = i;
        break;
      }
    }

    if (directorio_index == -1) {
      cout << "Error: Directorio raíz lleno." << endl;
      return false;
    }

    int free_count = 0;
    for (int i = 0; i < NUM_CLUSTERS; i++) {
      if (FAT_TABLE[i] == FAT_LIBRE)
        free_count++;
    }

    if (free_count < clusters_needed) {
      cout << "Error: No hay suficientes clusters libres." << endl;
      return false;
    }

    int last_cluster = -1;
    int start_cluster = -1;

    for (int k = 0; k < clusters_needed; k++) {
      int current_cluster = FindFreeCluster();

      if (current_cluster == -1) {
        if (start_cluster != -1) {
          DeallocateFile(start_cluster);
        }
        return false;
      }

      if (k == 0) {
        start_cluster = current_cluster;
      } else {
        FAT_TABLE[last_cluster] = current_cluster;
      }
      // std::cout << filename << " " << start_cluster << " " << current_cluster
      // << '\n';
      FAT_TABLE[current_cluster] = FAT_OCUPADO;
      last_cluster = current_cluster;
    }

    FAT_TABLE[last_cluster] = FAT_EOF;

    raiz[directorio_index].fileName = filename;
    raiz[directorio_index].size = contenido;
    raiz[directorio_index].start_cluster = start_cluster;
    raiz[directorio_index].isActive = true;

    cout << "Archivo '" << filename << "' asignado correctamente." << endl;
    cout << "Tamaño: " << contenido << " bytes, Clusters: " << clusters_needed
         << endl;
    return true;
  }

  bool DeleteFile(string filename) {
    int file_index = -1;
    for (int i = 0; i < MAX_FILES; i++) {
      if (raiz[i].isActive && raiz[i].fileName == filename) {
        file_index = i;
        break;
      }
    }

    if (file_index == -1) {
      cout << "Error: Archivo '" << filename << "' no encontrado." << endl;
      return false;
    }

    DeallocateFile(raiz[file_index].start_cluster);

    raiz[file_index].fileName = "";
    raiz[file_index].size = 0;
    raiz[file_index].start_cluster = -1;
    raiz[file_index].isActive = false;

    cout << "Archivo '" << filename << "' eliminado correctamente." << endl;
    return true;
  }

  void DeallocateFile(int start_cluster) {
    int current_cluster = start_cluster;

    while (current_cluster != FAT_EOF && current_cluster >= 0) {
      int next_cluster = FAT_TABLE[current_cluster];
      FAT_TABLE[current_cluster] = FAT_LIBRE;
      current_cluster = next_cluster;
    }
  }

  void DisplayFAT() {
    cout << "\nTABLA FAT\n";
    cout << left << setw(10) << "Cluster" << setw(15) << "| FAT_TABLE[i]"
         << " | Estado / Puntero" << endl;
    cout << "------------------------------------------------------------------"
            "--"
         << endl;

    for (int i = 0; i < NUM_CLUSTERS; i++) {
      cout << left << setw(10) << i;

      int valor = FAT_TABLE[i];

      if (valor == FAT_LIBRE) {
        cout << "| 0            " << setw(15);
        cout << " | LIBRE";
      } else if (valor == FAT_EOF) {
        cout << "|-1            " << setw(15);
        cout << " | FIN DE ARCHIVO (EOF)";
      } else if (valor == FAT_OCUPADO) {
        cout << "| -2 OCUPADO";
        cout << " | SIGUIENTE CLUSTER -> " << valor;
      } else {
        cout << "| " << valor << "            " << setw(15);
        cout << " | SIGUIENTE CLUSTER -> " << valor;
      }
      cout << endl;
    }
    cout << "=================================================================="
            "=\n";
  }

  void DisplayDirectory() {
    cout << "\n===== DIRECTORIO RAÍZ =====" << endl;
    cout << left << setw(20) << "Nombre" << setw(10) << "Tamaño" << setw(10)
         << "Inicio" << endl;
    cout << "-------------------------------------------" << endl;

    bool empty = true;
    for (int i = 0; i < MAX_FILES; i++) {
      if (raiz[i].isActive) {
        cout << left << setw(20) << raiz[i].fileName << setw(10) << raiz[i].size
             << setw(10) << raiz[i].start_cluster << endl;
        empty = false;
      }
    }

    if (empty) {
      cout << "(Directorio vacío)" << endl;
    }
    cout << "===========================\n" << endl;
  }

  void DisplayStats() {
    int clusters_libres = 0;
    int clusters_ocupados = 0;

    for (int i = 0; i < NUM_CLUSTERS; i++) {
      if (FAT_TABLE[i] == FAT_LIBRE) {
        clusters_libres++;
      } else {
        clusters_ocupados++;
      }
    }

    cout << "\n===== ESTADÍSTICAS =====" << endl;
    cout << "Total clusters: " << NUM_CLUSTERS << endl;
    cout << "Clusters libres: " << clusters_libres << endl;
    cout << "Clusters ocupados: " << clusters_ocupados << endl;
    cout << "Espacio libre: " << (clusters_libres * CLUSTER_SIZE) << " bytes"
         << endl;
    cout << "Espacio ocupado: " << (clusters_ocupados * CLUSTER_SIZE)
         << " bytes" << endl;
    cout << "========================\n" << endl;
  }
};

void prueba_01() {
  FATSystem fat;

  cout << "\n\n--- PASO 1: Creando Archivos Iniciales ---" << endl;
  fat.AllocateFile("DOC_A.TXT", 2500);
  fat.AllocateFile("IMG_B.JPG", 1500);
  fat.AllocateFile("PROJ_C.ZIP", 3500);

  fat.DisplayDirectory();
  fat.DisplayFAT();

  cout << "\n\n--- PASO 2: Eliminando el Archivo Central ---" << endl;
  fat.DeleteFile("IMG_B.JPG");
  fat.DisplayDirectory();
  fat.DisplayFAT();

  cout << "\n\n--- PASO 3: Creando nuevo Archivo (D) que rellena el hueco ---"
       << endl;
  fat.AllocateFile("DATA_D.LOG", 2800);

  fat.DisplayDirectory();
  fat.DisplayFAT();

  cout << "\n\n--- PASO 4: Limpieza Final ---" << endl;
  fat.DeleteFile("DOC_A.TXT");
  fat.DeleteFile("PROJ_C.ZIP");
  fat.DeleteFile("DATA_D.LOG");

  fat.DisplayDirectory();
  fat.DisplayFAT();
}

void menu_dinamico() {
  FATSystem fat;
  int opcion;

  do {
    cout << "\n========== SISTEMA FAT ==========" << endl;
    cout << "1. Crear archivo" << endl;
    cout << "2. Eliminar archivo" << endl;
    cout << "3. Mostrar directorio" << endl;
    cout << "4. Mostrar tabla FAT" << endl;
    cout << "5. Mostrar estadísticas" << endl;
    cout << "6. Reinicializar sistema" << endl;
    cout << "0. Salir" << endl;
    cout << "=================================" << endl;
    cout << "Opción: ";
    cin >> opcion;

    switch (opcion) {
    case 1: {
      string nombre;
      int tamanio;
      cout << "Nombre del archivo: ";
      cin >> nombre;
      cout << "Tamaño (bytes): ";
      cin >> tamanio;
      fat.AllocateFile(nombre, tamanio);
      break;
    }
    case 2: {
      string nombre;
      cout << "Nombre del archivo a eliminar: ";
      cin >> nombre;
      fat.DeleteFile(nombre);
      break;
    }
    case 3:
      fat.DisplayDirectory();
      break;
    case 4:
      fat.DisplayFAT();
      break;
    case 5:
      fat.DisplayStats();
      break;
    case 6:
      fat.InitializeFAT();
      break;
    case 0:
      cout << "Saliendo del sistema..." << endl;
      break;
    default:
      cout << "Opción inválida." << endl;
    }
  } while (opcion != 0);
}
int main() {
  // menu_dinamico();
  prueba_01();
  return 0;
}
