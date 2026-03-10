# Guide de Démarrage Rapide - Refactorisation

## 🎯 Vue d'Ensemble en 5 Minutes

### Problème Principal
**Votre code actuel mélange tout**: réseau + HTTP + business logic + configuration dans les mêmes classes.

### Solution
**Séparer en couches distinctes** avec des responsabilités claires.

---

## 📊 Architecture Visuelle

```
┌─────────────────────────────────────────────────────────────┐
│                         SERVER                               │
│  (Orchestration uniqueent - 150 lignes)                     │
└──────────────────────────┬──────────────────────────────────┘
                           │
        ┌──────────────────┼──────────────────┐
        │                  │                  │
        ▼                  ▼                  ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│  NETWORK     │  │     HTTP     │  │ APPLICATION  │
│   LAYER      │  │    LAYER     │  │    LAYER     │
│              │  │              │  │              │
│ IOMultiplexer│  │  HttpParser  │  │   Handlers   │
│  TcpSocket   │  │  HttpRouter  │  │ (Static/CGI) │
│ SocketManager│  │  Validator   │  │  Processor   │
│ Connection   │  │  Builder     │  │              │
└──────────────┘  └──────────────┘  └──────────────┘
        │                  │                  │
        └──────────────────┴──────────────────┘
                           │
                           ▼
                  ┌──────────────┐
                  │   UTILITY    │
                  │    LAYER     │
                  │              │
                  │  FileSystem  │
                  │  PathUtils   │
                  │ MimeResolver │
                  │    Logger    │
                  └──────────────┘
```

---

## 🔥 Top 5 des Changements

### 1. Server.cpp: De 265 lignes → 150 lignes
**AVANT**:
```cpp
void Server::handlePollEvents() {
    // 150+ lignes qui font TOUT:
    // - Gestion listen sockets
    // - Gestion client connections
    // - Gestion CGI pipes
    // - State management
    // - Error handling
    // 😱 IMPOSSIBLE À MAINTENIR
}
```

**APRÈS**:
```cpp
void Server::handleEvents() {
    std::vector<IOMultiplexer::Event> events = _multiplexer->getEvents();
    
    for (size_t i = 0; i < events.size(); ++i) {
        if (_socketManager->isListenSocket(events[i].fd))
            handleListenSocket(events[i].fd);
        else if (_connections.count(events[i].fd))
            handleClientEvent(events[i]);
        else if (_cgiExecutors.count(events[i].fd))
            handleCgiEvent(events[i]);
    }
    // ✅ CLAIR ET MAINTENABLE
}
```

### 2. Connection: Séparer I/O et Logique Métier
**AVANT**:
```cpp
void Connection::processRequest() {
    // 100+ lignes qui font:
    // - Parsing
    // - Routing
    // - Validation
    // - CGI detection
    // - GET/POST/DELETE logic
    // - Error handling
    // 😱 TROP DE RESPONSABILITÉS
}
```

**APRÈS**:
```cpp
// ClientConnection: juste I/O
bool ClientConnection::readFromSocket() {
    // Juste lire les données
}

// RequestProcessor: logique métier
HttpResponse RequestProcessor::process(const HttpRequest& req, const ServerConfig* cfg) {
    const Config* config = _router->resolveLocation(req.path);
    IRequestHandler* handler = findHandler(req, config);
    return handler->handle(req, config);
    // ✅ SÉPARATION CLAIRE
}
```

### 3. HttpResponse: Builder Pattern
**AVANT**:
```cpp
// Plein de static methods complexes
static HttpResponse makeGetResponse(const std::string& path, const Config* config) {
    // 50+ lignes de logique mélangée
}
```

**APRÈS**:
```cpp
HttpResponseBuilder builder;
return builder
    .setStatus(200)
    .setContentType("text/html")
    .setBody(content)
    .build();
// ✅ FLUENT API, CLAIR
```

### 4. CGI: Séparation Execution / Parsing
**AVANT**:
```cpp
class CgiHandler {
    // 459 lignes qui font:
    // - Fork/exec
    // - Pipe management
    // - I/O non-bloquant
    // - Response parsing
    // - Timeout checking
    // 😱 MONSTRE
};
```

**APRÈS**:
```cpp
class CgiExecutor {
    // 150 lignes: juste exécution et I/O
    void execute(...);
    bool readOutput();
    bool writeInput();
};

class CgiResponseParser {
    // 80 lignes: juste parsing
    CgiResponse parse(const std::string& output);
};

class CgiHandler {
    // 60 lignes: orchestration
    HttpResponse handle(const HttpRequest& req, const Config* cfg) {
        _executor->execute(...);
        // ... wait ...
        return _parser->toHttpResponse(_executor->getOutput());
    }
};
// ✅ SÉPARATION CLAIRE
```

### 5. Config: Registre de Directives
**AVANT**:
```cpp
void Config::parseGlobalDirective(const std::string& directive) {
    // 200+ lignes avec map de pointeurs de fonctions
    // Code répétitif pour chaque directive
    // 😱 DIFFICILE À ÉTENDRE
}
```

**APRÈS**:
```cpp
// Enregistrement
DirectiveRegistry registry;
registry.registerDirective("listen", new ListenDirective());
registry.registerDirective("root", new RootDirective());
// ...

// Utilisation
registry.applyDirective("listen", config, args);

// Ajouter une directive = créer UNE classe
class ListenDirective : public IDirectiveHandler {
    void apply(Config& config, const std::vector<std::string>& args) {
        // 10-20 lignes
    }
};
// ✅ EXTENSIBLE, OCP
```

---

## 🚦 Par Où Commencer?

### Option A: Approche Bottom-Up (Recommandée)
**Commencer par les utilitaires, remonter vers Server**

```
Semaine 1: Utility Layer
├── Jour 1-2: FileSystem, PathUtils
├── Jour 3: MimeTypeResolver
└── Jour 4-5: Tests + Migration FileHandler

Semaine 2: Network Layer
├── Jour 1: TcpSocket
├── Jour 2: IOMultiplexer
├── Jour 3-4: SocketManager, ClientConnection
└── Jour 5: Tests + Migration partielle Server

Semaine 3: HTTP Layer + Simple Handlers
├── Jour 1-2: HttpParser, Validator
├── Jour 3: HttpRouter, ResponseBuilder
├── Jour 4: StaticFileHandler, DirectoryListingHandler
└── Jour 5: Tests + Migration Connection

Semaine 4: CGI + Config + Integration
├── Jour 1-2: CgiExecutor, CgiResponseParser
├── Jour 3: ConfigParser refactoring
├── Jour 4: Integration complète
└── Jour 5: Tests finaux et cleanup
```

### Option B: Approche Par Feature
**Refactoriser une fonctionnalité à la fois**

```
Feature 1: Static Files (4-5 jours)
└── FileSystem → PathUtils → StaticFileHandler → Tests

Feature 2: CGI (5-6 jours)
└── CgiExecutor → CgiParser → CgiHandler → Tests

Feature 3: Config (4-5 jours)
└── ConfigParser → DirectiveHandlers → Tests

Feature 4: Network (5-6 jours)
└── IOMultiplexer → TcpSocket → SocketManager → Tests

Feature 5: Integration (3-4 jours)
└── Assembler tout + Tests d'intégration
```

---

## 📝 Checklist de Démarrage

### Avant de Commencer
- [ ] Sauvegarder le code actuel (branch `main-backup`)
- [ ] Créer une branche de développement (`refactor/architecture`)
- [ ] S'assurer que les tests actuels passent
- [ ] Lire le plan complet (REFACTORING_PLAN.md)
- [ ] Choisir une approche (Bottom-Up ou Par Feature)

### Pour Chaque Nouvelle Classe
- [ ] Une seule responsabilité
- [ ] Interface claire (méthodes publiques documentées)
- [ ] Pas de dépendances cachées
- [ ] Injection de dépendances
- [ ] Tests unitaires
- [ ] Documentation

### Tests de Validation
- [ ] Tests unitaires pour chaque composant
- [ ] Tests d'intégration pour les workflows complets
- [ ] Tests avec le tester officiel (si disponible)
- [ ] Tests de non-régression
- [ ] Tests de performance (pas de dégradation)

---

## 🎓 Exemple Complet: Ajouter un Handler

### Ancien Code (Modification difficile)
```cpp
// Il faut modifier Connection.cpp
void Connection::processRequest() {
    // ... 100 lignes ...
    if (_request.method == "GET") {
        // logique GET
    } else if (_request.method == "POST") {
        // logique POST
    } else if (_request.method == "DELETE") {
        // logique DELETE
    }
    // Pour ajouter OPTIONS, il faut modifier ici
    // Risque de casser GET/POST/DELETE 😱
}
```

### Nouveau Code (Extension facile)
```cpp
// 1. Créer une nouvelle classe
class OptionsHandler : public IRequestHandler {
public:
    bool canHandle(const HttpRequest& req, const Config* cfg) const {
        return req.method == "OPTIONS";
    }
    
    HttpResponse handle(const HttpRequest& req, const Config* cfg) {
        HttpResponseBuilder builder;
        return builder
            .setStatus(200)
            .setHeader("Allow", "GET, POST, DELETE, OPTIONS")
            .build();
    }
};

// 2. Enregistrer le handler
_processor->registerHandler(new OptionsHandler());

// ✅ Aucune modification du code existant!
// ✅ Open/Closed Principle respecté!
```

---

## 🧪 Template de Test Unitaire

```cpp
#include "gtest/gtest.h"  // ou votre framework de test
#include "FileSystem.hpp"

class FileSystemTest : public ::testing::Test {
protected:
    FileSystem* fs;
    
    void SetUp() override {
        fs = new FileSystem();
        // Setup du test
    }
    
    void TearDown() override {
        delete fs;
        // Cleanup
    }
};

TEST_F(FileSystemTest, ExistsReturnsTrueForExistingFile) {
    // Arrange
    std::string path = "/tmp/test_file.txt";
    fs->writeFile(path, "test content");
    
    // Act
    bool exists = fs->exists(path);
    
    // Assert
    EXPECT_TRUE(exists);
    
    // Cleanup
    fs->deleteFile(path);
}

TEST_F(FileSystemTest, ReadFileReturnsCorrectContent) {
    // Arrange
    std::string path = "/tmp/test_file.txt";
    std::string expected = "Hello, World!";
    fs->writeFile(path, expected);
    
    // Act
    std::string actual = fs->readFile(path);
    
    // Assert
    EXPECT_EQ(expected, actual);
    
    // Cleanup
    fs->deleteFile(path);
}
```

---

## 💡 Tips & Best Practices

### 1. **Gardez les Classes Petites**
- Une classe = une responsabilité
- Moins de 200 lignes par classe
- Moins de 30 lignes par méthode

### 2. **Nommage Clair**
```cpp
// ❌ Mauvais
void process(std::string s);

// ✅ Bon
void parseHttpRequest(const std::string& rawRequest);
```

### 3. **Injection de Dépendances**
```cpp
// ❌ Mauvais (dépendance cachée)
class Handler {
    FileSystem fs;  // Création interne
};

// ✅ Bon (injection)
class Handler {
    FileSystem* _fs;
public:
    Handler(FileSystem* fs) : _fs(fs) {}
};
```

### 4. **RAII Toujours**
```cpp
// ❌ Mauvais
int fd = open(...);
// ... beaucoup de code avec des returns possibles ...
close(fd);  // Peut ne jamais être appelé!

// ✅ Bon
FdGuard fd(open(...));
// ... code ...
// Fermeture automatique, même en cas d'exception
```

### 5. **Const Correctness**
```cpp
// Toujours const si possible
class FileSystem {
public:
    bool exists(const std::string& path) const;  // ✅
    std::string readFile(const std::string& path);  // ✅ (pas const car peut lever exception)
};
```

---

## 📊 Métriques de Succès

### Code Quality
- [ ] Cyclomatic Complexity < 10 par fonction
- [ ] Couplage afférent/efférent < 5
- [ ] Code Coverage > 80%
- [ ] Zero memory leaks (valgrind)

### Performance
- [ ] Même performance ou mieux
- [ ] Pas de régression sur les benchmarks
- [ ] Temps de réponse stables

### Maintenabilité
- [ ] Temps pour ajouter une feature: < 1 jour
- [ ] Temps pour fixer un bug: < 2 heures
- [ ] Onboarding nouveau dev: < 1 semaine

---

## 🆘 FAQ

### Q: Dois-je vraiment tout refactoriser?
**R**: Oui, mais progressivement. L'architecture actuelle n'est pas maintenable à long terme.

### Q: Combien de temps ça va prendre?
**R**: 3-4 semaines en travaillant régulièrement. C'est un investissement qui sera rentabilisé rapidement.

### Q: Et si je casse quelque chose?
**R**: C'est pour ça qu'on a des tests! Écrivez des tests avant de refactoriser, et validez après chaque changement.

### Q: Puis-je faire plus simple?
**R**: Oui, mais vous aurez les mêmes problèmes qu'actuellement. Cette architecture est le minimum pour un code maintenable.

### Q: Ça va améliorer les performances?
**R**: Probablement pas significativement. Le but est la maintenabilité, pas la performance. Mais le code sera plus facile à optimiser.

### Q: Dois-je utiliser C++11/14/17?
**R**: Utilisez ce qui est autorisé par votre projet (probablement C++98). L'architecture fonctionne avec n'importe quelle version.

---

## 📚 Ressources Complémentaires

### Lecture Recommandée
- **Clean Code** by Robert C. Martin
- **Design Patterns** by Gang of Four
- **Effective C++** by Scott Meyers

### Patterns à Connaître
- Strategy Pattern (IRequestHandler)
- Factory Pattern (HttpResponseBuilder)
- Registry Pattern (DirectiveRegistry)
- Chain of Responsibility (RequestProcessor)

### Principes SOLID
- **S**: Single Responsibility
- **O**: Open/Closed
- **L**: Liskov Substitution
- **I**: Interface Segregation
- **D**: Dependency Inversion

---

## 🎯 Objectif Final

```
CODE ACTUEL (Monolithe):
├── Difficile à comprendre
├── Difficile à modifier
├── Difficile à tester
├── Risque de bugs élevé
└── Maintenance cauchemardesque

        ⬇️ REFACTORING ⬇️

CODE REFACTORISÉ (Modulaire):
├── ✅ Facile à comprendre (responsabilités claires)
├── ✅ Facile à modifier (faible couplage)
├── ✅ Facile à tester (injection de dépendances)
├── ✅ Risque de bugs faible (isolation)
└── ✅ Maintenance agréable
```

---

**Courage! Le chemin est long mais le résultat en vaut la peine! 💪**

---

## 🚀 Commande pour Commencer

```bash
# 1. Sauvegarder
git checkout -b main-backup
git checkout main

# 2. Créer branche de travail
git checkout -b refactor/architecture

# 3. Créer la structure
mkdir -p include/network include/http include/application include/config include/utility
mkdir -p src/network src/http src/application src/config src/utility
mkdir -p tests/network tests/http tests/application tests/config tests/utility

# 4. Commencer par les utilitaires
touch include/utility/FileSystem.hpp
touch src/utility/FileSystem.cpp
# ... et c'est parti!
```

**Bon courage! 🚀**
