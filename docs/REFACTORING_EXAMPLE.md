# Exemple d'Implémentation - FileSystem

## 🎯 Objectif
Montrer concrètement comment implémenter une classe de la nouvelle architecture avec:
- Header propre
- Implémentation complète
- Tests unitaires
- Documentation

---

## 📁 Structure des Fichiers

```
webserv/
├── include/
│   └── utility/
│       └── FileSystem.hpp       ← Interface
├── src/
│   └── utility/
│       └── FileSystem.cpp       ← Implémentation
└── tests/
    └── utility/
        └── FileSystemTest.cpp   ← Tests
```

---

## 📄 Header File: include/utility/FileSystem.hpp

```cpp
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: you <you@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 by you                      #+#    #+#             */
/*   Updated: 2026/03/08 by you                     ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string>
#include <vector>
#include <ctime>

/**
 * @brief Encapsulation des opérations sur le système de fichiers
 * 
 * Cette classe fournit une abstraction pour les opérations courantes
 * sur les fichiers et répertoires, avec gestion d'erreurs cohérente.
 * 
 * Responsabilités:
 * - Tester l'existence et les propriétés des fichiers
 * - Lire et écrire des fichiers
 * - Lister les répertoires
 * - Supprimer des fichiers
 * 
 * Note: Toutes les méthodes sont non-statiques pour permettre
 * le mocking dans les tests (dependency injection).
 */
class FileSystem {
public:
    /**
     * @brief Exception levée lors d'erreurs d'I/O
     */
    class IOException : public std::exception {
    private:
        std::string _message;
    public:
        IOException(const std::string& message) : _message(message) {}
        virtual ~IOException() throw() {}
        virtual const char* what() const throw() { return _message.c_str(); }
    };

public:
    FileSystem();
    virtual ~FileSystem();

    // ==================== Test d'Existence ====================
    
    /**
     * @brief Vérifie si un chemin existe (fichier ou répertoire)
     * @param path Chemin à vérifier
     * @return true si le chemin existe, false sinon
     */
    virtual bool exists(const std::string& path) const;

    /**
     * @brief Vérifie si un chemin est un fichier régulier
     * @param path Chemin à vérifier
     * @return true si c'est un fichier, false sinon
     */
    virtual bool isFile(const std::string& path) const;

    /**
     * @brief Vérifie si un chemin est un répertoire
     * @param path Chemin à vérifier
     * @return true si c'est un répertoire, false sinon
     */
    virtual bool isDirectory(const std::string& path) const;

    /**
     * @brief Vérifie si un fichier est lisible
     * @param path Chemin du fichier
     * @return true si lisible, false sinon
     */
    virtual bool isReadable(const std::string& path) const;

    /**
     * @brief Vérifie si un fichier est modifiable
     * @param path Chemin du fichier
     * @return true si modifiable, false sinon
     */
    virtual bool isWritable(const std::string& path) const;

    // ==================== Lecture ====================

    /**
     * @brief Lit le contenu complet d'un fichier
     * @param path Chemin du fichier à lire
     * @return Contenu du fichier en tant que string
     * @throws IOException si le fichier n'existe pas ou n'est pas lisible
     */
    virtual std::string readFile(const std::string& path);

    /**
     * @brief Liste les entrées d'un répertoire
     * @param path Chemin du répertoire
     * @return Vector contenant les noms des entrées (sans . et ..)
     * @throws IOException si le répertoire n'existe pas ou n'est pas lisible
     */
    virtual std::vector<std::string> listDirectory(const std::string& path);

    // ==================== Écriture ====================

    /**
     * @brief Écrit du contenu dans un fichier (écrase si existant)
     * @param path Chemin du fichier
     * @param content Contenu à écrire
     * @return true si succès, false sinon
     */
    virtual bool writeFile(const std::string& path, const std::string& content);

    /**
     * @brief Supprime un fichier
     * @param path Chemin du fichier à supprimer
     * @return true si succès, false si échec ou si c'est un répertoire
     */
    virtual bool deleteFile(const std::string& path);

    /**
     * @brief Crée un répertoire
     * @param path Chemin du répertoire à créer
     * @return true si succès, false sinon
     * @note Ne crée pas les répertoires parents manquants
     */
    virtual bool createDirectory(const std::string& path);

    // ==================== Informations ====================

    /**
     * @brief Obtient la taille d'un fichier en octets
     * @param path Chemin du fichier
     * @return Taille en octets, ou 0 si erreur
     */
    virtual size_t getFileSize(const std::string& path) const;

    /**
     * @brief Obtient la date de dernière modification
     * @param path Chemin du fichier
     * @return Timestamp de dernière modification, ou 0 si erreur
     */
    virtual time_t getLastModified(const std::string& path) const;

private:
    // Interdit la copie (pas nécessaire pour cette classe)
    FileSystem(const FileSystem&);
    FileSystem& operator=(const FileSystem&);
};

#endif // FILESYSTEM_HPP
```

---

## 💻 Implementation: src/utility/FileSystem.cpp

```cpp
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: you <you@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 by you                      #+#    #+#             */
/*   Updated: 2026/03/08 by you                     ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/FileSystem.hpp"

#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstdio>

// ==================== Constructeurs/Destructeur ====================

FileSystem::FileSystem() {
}

FileSystem::~FileSystem() {
}

// ==================== Test d'Existence ====================

bool FileSystem::exists(const std::string& path) const {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool FileSystem::isFile(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISREG(st.st_mode);
}

bool FileSystem::isDirectory(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

bool FileSystem::isReadable(const std::string& path) const {
    return access(path.c_str(), R_OK) == 0;
}

bool FileSystem::isWritable(const std::string& path) const {
    return access(path.c_str(), W_OK) == 0;
}

// ==================== Lecture ====================

std::string FileSystem::readFile(const std::string& path) {
    // Vérifier que le fichier existe et est lisible
    if (!exists(path)) {
        throw IOException("File does not exist: " + path);
    }
    
    if (!isFile(path)) {
        throw IOException("Path is not a file: " + path);
    }
    
    if (!isReadable(path)) {
        throw IOException("File is not readable: " + path);
    }

    // Ouvrir le fichier en mode binaire
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        throw IOException("Failed to open file: " + path + 
                          " (" + std::string(std::strerror(errno)) + ")");
    }

    // Lire tout le contenu
    std::ostringstream ss;
    ss << file.rdbuf();
    
    if (file.bad()) {
        throw IOException("Error reading file: " + path);
    }
    
    file.close();
    return ss.str();
}

std::vector<std::string> FileSystem::listDirectory(const std::string& path) {
    // Vérifier que c'est un répertoire
    if (!exists(path)) {
        throw IOException("Directory does not exist: " + path);
    }
    
    if (!isDirectory(path)) {
        throw IOException("Path is not a directory: " + path);
    }

    // Ouvrir le répertoire
    DIR* dir = opendir(path.c_str());
    if (!dir) {
        throw IOException("Failed to open directory: " + path +
                          " (" + std::string(std::strerror(errno)) + ")");
    }

    // Lire les entrées
    std::vector<std::string> entries;
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL) {
        std::string name = entry->d_name;
        
        // Ignorer . et ..
        if (name != "." && name != "..") {
            entries.push_back(name);
        }
    }

    closedir(dir);
    
    // Trier par ordre alphabétique pour cohérence
    std::sort(entries.begin(), entries.end());
    
    return entries;
}

// ==================== Écriture ====================

bool FileSystem::writeFile(const std::string& path, const std::string& content) {
    // Ouvrir le fichier en mode binaire (écrase si existe)
    std::ofstream file(path.c_str(), std::ios::out | std::ios::binary);
    if (!file) {
        return false;
    }

    // Écrire le contenu
    file.write(content.c_str(), content.size());
    
    if (file.bad()) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool FileSystem::deleteFile(const std::string& path) {
    // Vérifier que le fichier existe
    if (!exists(path)) {
        return false;
    }
    
    // Ne supprimer que les fichiers, pas les répertoires
    if (!isFile(path)) {
        return false;
    }

    // Supprimer le fichier
    return std::remove(path.c_str()) == 0;
}

bool FileSystem::createDirectory(const std::string& path) {
    // mkdir avec permissions 0755
    return mkdir(path.c_str(), 0755) == 0;
}

// ==================== Informations ====================

size_t FileSystem::getFileSize(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return static_cast<size_t>(st.st_size);
}

time_t FileSystem::getLastModified(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_mtime;
}
```

---

## 🧪 Tests: tests/utility/FileSystemTest.cpp

```cpp
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystemTest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: you <you@student.42.fr>                    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 by you                      #+#    #+#             */
/*   Updated: 2026/03/08 by you                     ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../include/utility/FileSystem.hpp"

#include <cassert>
#include <iostream>
#include <cstdlib>

// Simple test framework (si vous n'avez pas gtest)
#define TEST(name) void test_##name()
#define ASSERT_TRUE(expr) assert(expr)
#define ASSERT_FALSE(expr) assert(!(expr))
#define ASSERT_EQ(a, b) assert((a) == (b))
#define ASSERT_THROW(expr, exception) \
    do { \
        bool caught = false; \
        try { expr; } \
        catch (const exception&) { caught = true; } \
        assert(caught); \
    } while(0)

// ==================== Test Helpers ====================

static std::string getTempPath() {
    char tmpl[] = "/tmp/webserv_test_XXXXXX";
    return std::string(mktemp(tmpl));
}

static void cleanup(const std::string& path) {
    std::remove(path.c_str());
}

// ==================== Tests ====================

TEST(ExistsReturnsFalseForNonExistentFile) {
    FileSystem fs;
    ASSERT_FALSE(fs.exists("/this/file/does/not/exist"));
    std::cout << "✓ ExistsReturnsFalseForNonExistentFile" << std::endl;
}

TEST(ExistsReturnsTrueForExistingFile) {
    FileSystem fs;
    std::string path = getTempPath();
    
    // Créer le fichier
    fs.writeFile(path, "test");
    
    // Test
    ASSERT_TRUE(fs.exists(path));
    
    // Cleanup
    cleanup(path);
    std::cout << "✓ ExistsReturnsTrueForExistingFile" << std::endl;
}

TEST(IsFileReturnsTrueForRegularFile) {
    FileSystem fs;
    std::string path = getTempPath();
    
    fs.writeFile(path, "test");
    ASSERT_TRUE(fs.isFile(path));
    ASSERT_FALSE(fs.isDirectory(path));
    
    cleanup(path);
    std::cout << "✓ IsFileReturnsTrueForRegularFile" << std::endl;
}

TEST(IsDirectoryReturnsTrueForDirectory) {
    FileSystem fs;
    
    // /tmp devrait toujours exister
    ASSERT_TRUE(fs.isDirectory("/tmp"));
    ASSERT_FALSE(fs.isFile("/tmp"));
    
    std::cout << "✓ IsDirectoryReturnsTrueForDirectory" << std::endl;
}

TEST(WriteAndReadFile) {
    FileSystem fs;
    std::string path = getTempPath();
    std::string content = "Hello, World!\nLine 2\nLine 3";
    
    // Écrire
    ASSERT_TRUE(fs.writeFile(path, content));
    
    // Lire
    std::string read = fs.readFile(path);
    ASSERT_EQ(content, read);
    
    cleanup(path);
    std::cout << "✓ WriteAndReadFile" << std::endl;
}

TEST(ReadFileThrowsForNonExistentFile) {
    FileSystem fs;
    
    ASSERT_THROW(
        fs.readFile("/this/file/does/not/exist"),
        FileSystem::IOException
    );
    
    std::cout << "✓ ReadFileThrowsForNonExistentFile" << std::endl;
}

TEST(ReadFileThrowsForDirectory) {
    FileSystem fs;
    
    ASSERT_THROW(
        fs.readFile("/tmp"),
        FileSystem::IOException
    );
    
    std::cout << "✓ ReadFileThrowsForDirectory" << std::endl;
}

TEST(DeleteFileRemovesFile) {
    FileSystem fs;
    std::string path = getTempPath();
    
    // Créer le fichier
    fs.writeFile(path, "test");
    ASSERT_TRUE(fs.exists(path));
    
    // Supprimer
    ASSERT_TRUE(fs.deleteFile(path));
    ASSERT_FALSE(fs.exists(path));
    
    std::cout << "✓ DeleteFileRemovesFile" << std::endl;
}

TEST(DeleteFileReturnsFalseForNonExistent) {
    FileSystem fs;
    ASSERT_FALSE(fs.deleteFile("/this/file/does/not/exist"));
    std::cout << "✓ DeleteFileReturnsFalseForNonExistent" << std::endl;
}

TEST(GetFileSizeReturnsCorrectSize) {
    FileSystem fs;
    std::string path = getTempPath();
    std::string content = "Hello";  // 5 bytes
    
    fs.writeFile(path, content);
    ASSERT_EQ(5u, fs.getFileSize(path));
    
    cleanup(path);
    std::cout << "✓ GetFileSizeReturnsCorrectSize" << std::endl;
}

TEST(ListDirectoryReturnsEntries) {
    FileSystem fs;
    std::string tempDir = "/tmp/webserv_test_dir";
    
    // Créer répertoire et fichiers
    fs.createDirectory(tempDir);
    fs.writeFile(tempDir + "/file1.txt", "test1");
    fs.writeFile(tempDir + "/file2.txt", "test2");
    
    // Lister
    std::vector<std::string> entries = fs.listDirectory(tempDir);
    
    // Vérifier (devrait être trié)
    ASSERT_EQ(2u, entries.size());
    ASSERT_EQ("file1.txt", entries[0]);
    ASSERT_EQ("file2.txt", entries[1]);
    
    // Cleanup
    fs.deleteFile(tempDir + "/file1.txt");
    fs.deleteFile(tempDir + "/file2.txt");
    rmdir(tempDir.c_str());
    
    std::cout << "✓ ListDirectoryReturnsEntries" << std::endl;
}

TEST(ListDirectoryThrowsForNonExistent) {
    FileSystem fs;
    
    ASSERT_THROW(
        fs.listDirectory("/this/directory/does/not/exist"),
        FileSystem::IOException
    );
    
    std::cout << "✓ ListDirectoryThrowsForNonExistent" << std::endl;
}

// ==================== Test Runner ====================

int main() {
    std::cout << "\n=== FileSystem Tests ===\n" << std::endl;
    
    try {
        test_ExistsReturnsFalseForNonExistentFile();
        test_ExistsReturnsTrueForExistingFile();
        test_IsFileReturnsTrueForRegularFile();
        test_IsDirectoryReturnsTrueForDirectory();
        test_WriteAndReadFile();
        test_ReadFileThrowsForNonExistentFile();
        test_ReadFileThrowsForDirectory();
        test_DeleteFileRemovesFile();
        test_DeleteFileReturnsFalseForNonExistent();
        test_GetFileSizeReturnsCorrectSize();
        test_ListDirectoryReturnsEntries();
        test_ListDirectoryThrowsForNonExistent();
        
        std::cout << "\n✅ All tests passed!\n" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed: " << e.what() << "\n" << std::endl;
        return 1;
    }
}
```

---

## 🔨 Compilation et Exécution

### Makefile pour les tests

```makefile
# Tests FileSystem
test_filesystem: tests/utility/FileSystemTest.cpp src/utility/FileSystem.cpp
	$(CXX) $(CXXFLAGS) -I include -o test_filesystem \
		tests/utility/FileSystemTest.cpp \
		src/utility/FileSystem.cpp
	./test_filesystem

.PHONY: test_filesystem
```

### Commandes

```bash
# Compiler
make test_filesystem

# Ou manuellement
g++ -Wall -Wextra -Werror -std=c++98 -I include \
    -o test_filesystem \
    tests/utility/FileSystemTest.cpp \
    src/utility/FileSystem.cpp

# Exécuter
./test_filesystem

# Avec valgrind pour vérifier les fuites
valgrind --leak-check=full ./test_filesystem
```

---

## 📝 Comment Utiliser FileSystem

### Migration depuis FileHandler

**AVANT (FileHandler.cpp)**:
```cpp
// Code actuel
if (FileHandler::fileExists(path)) {
    std::string content = FileHandler::readFile(path);
    // ...
}
```

**APRÈS (avec FileSystem)**:
```cpp
// Nouveau code
FileSystem fs;

if (fs.exists(path) && fs.isFile(path)) {
    try {
        std::string content = fs.readFile(path);
        // ...
    } catch (const FileSystem::IOException& e) {
        // Gérer l'erreur
        std::cerr << "Error: " << e.what() << std::endl;
    }
}
```

### Injection de Dépendances

```cpp
// Dans StaticFileHandler
class StaticFileHandler {
private:
    FileSystem* _fs;  // Injection

public:
    StaticFileHandler(FileSystem* fs) : _fs(fs) {}
    
    HttpResponse serveFile(const std::string& path) {
        if (!_fs->exists(path)) {
            return makeErrorResponse(404);
        }
        
        try {
            std::string content = _fs->readFile(path);
            return makeSuccessResponse(content);
        } catch (const FileSystem::IOException& e) {
            return makeErrorResponse(500);
        }
    }
};

// Utilisation
FileSystem fs;
StaticFileHandler handler(&fs);
```

---

## ✅ Checklist de Qualité

### Header File
- [x] Include guards
- [x] Documentation des méthodes
- [x] Méthodes const quand approprié
- [x] Méthodes virtual pour permettre le mocking
- [x] Exception personnalisée
- [x] Pas de dépendances inutiles

### Implementation
- [x] Gestion d'erreurs complète
- [x] Vérifications de sécurité
- [x] Pas de memory leaks
- [x] Code clair et lisible
- [x] Commentaires pour les parties complexes

### Tests
- [x] Test des cas normaux
- [x] Test des cas d'erreur
- [x] Test des cas limites
- [x] Cleanup après chaque test
- [x] Tests isolés (pas de dépendances entre tests)

---

## 🎯 Prochaines Étapes

Après avoir implémenté FileSystem, vous pouvez:

1. **Implémenter PathUtils** (utilise les mêmes principes)
2. **Implémenter MimeTypeResolver** (très similaire)
3. **Créer StaticFileHandler** (utilise FileSystem et MimeTypeResolver)
4. **Continuer avec les autres classes de l'architecture**

---

## 💡 Conseils Finaux

### Gardez le Même Style
- Utilisez le même style de documentation
- Même structure de fichiers
- Même approche pour les tests
- Cohérence dans tout le projet

### Testez Chaque Composant
- N'attendez pas d'avoir tout implémenté pour tester
- Tests unitaires pour chaque classe
- Tests d'intégration ensuite

### Documentez Votre Code
- Commentaires pour les algorithmes complexes
- Documentation des interfaces publiques
- README pour expliquer l'architecture

---

**Vous avez maintenant un exemple complet à suivre! 🚀**

**Pour chaque nouvelle classe:**
1. Créer le header avec documentation
2. Implémenter les méthodes
3. Écrire les tests
4. Valider avec valgrind
5. Passer à la classe suivante

**Bon courage! 💪**
