# Git Strategy & Pull Request Plan

## Current Situation

**Current Branch:** `feat/Observer-Pattern`  
**Status:** Up to date with origin + many unstaged changes  
**Total Changes:** 
- Modified: 28 files
- Untracked: docs/ (3 files), simulator/ (5 files)

---

## File Categorization & Branch Strategy

### Branch 1: `refactor/separate-header-implementation`

**Purpose:** Refactor C++ code to follow OOP best practices by separating header declarations from implementations

**Files to Include:**

**New Files (Untracked):**
```
src/adapters/abstract_adapters/observable/observable.cpp
src/handlers/sensor_data_log_handler/sensor_data_log_handler.cpp
src/handlers/sensor_data_validator/sensor_data_validator.cpp
```

**Modified Files:**
```
src/adapters/abstract_adapters/observable/observable.h
src/handlers/sensor_data_log_handler/sensor_data_log_handler.h
src/handlers/sensor_data_validator/sensor_data_validator.h
```

**Commit Message:**
```
refactor: separate header and implementation files for OOP compliance

- Create .cpp files for Observable, SensorDataLogHandler, and SensorDataValidator
- Move implementations from headers to separate compilation units
- Keep only declarations in header files
- Improves compile time and follows C++ best practices
- All concrete classes now have proper header/implementation separation

This change makes the codebase more maintainable by:
- Reducing recompilation when implementation changes
- Making interfaces clearer (header shows contract only)
- Following industry standard C++ project structure
```

---

### Branch 2: `docs/architecture-guide`

**Purpose:** Add comprehensive architecture documentation

**Files to Include:**

**New Files (Untracked):**
```
docs/ARCHITECTURE.md
docs/GIT_STRATEGY.md
docs/iot-integration-project-architecture.md
```

**Modified Files:**
```
README.md (if documentation links added)
```

**Commit Message:**
```
docs: add comprehensive architecture and workflow documentation

- Add ARCHITECTURE.md with detailed system design explanation
- Include design pattern descriptions (Observer, Bridge, Adapter)
- Add data flow diagrams and component interactions
- Provide step-by-step guide for understanding codebase
- Include practical examples and use cases

Documentation covers:
- Overall data flow from gRPC to WebSocket/DDS
- Design patterns with detailed explanations
- Folder structure and component responsibilities
- How to understand and navigate the codebase
- Request-response flow scenarios
```

---

### Branch 3: `chore/build-config-improvements`

**Purpose:** Improve build configuration and development environment

**Files to Include:**

**Modified Files:**
```
CMakeLists.txt
.gitignore
```

**Commit Message:**
```
chore: improve CMake configuration and gitignore

CMakeLists.txt improvements:
- Add conditional checks for OpenDDS environment variables
- Prevent empty-string include directory errors
- Support building without env vars pre-exported
- Better error handling for missing dependencies
- Add detailed comments explaining each section

.gitignore updates:
- Add Conan-generated CMake files
- Add clang tools artifacts (.clangd, .cache, .ccls)
- Add build artifacts (CPackConfig, DartConfiguration)
- Add Python and C++ development artifacts
- Add executable and library outputs
- Prevent accidental commit of generated files

These changes improve developer experience by:
- Making first-time build less error-prone
- Reducing repository pollution with generated files
- Supporting multiple IDEs and development environments
```

---

### Branch 4: `feat/Observer-Pattern` (Current - Keep as is)

**Purpose:** Implementation of Observer Pattern for sensor data processing

**Files to Include:**

**Modified Files:**
```
src/app.cpp
src/controllers/sensor_controller.h
src/controllers/sensor_controller.cpp
src/adapters/service_adapters/bridge_manager.h
src/adapters/service_adapters/bridge_manager.cpp
src/adapters/service_adapters/dds_adapters/dds_adapter.h
src/adapters/service_adapters/dds_adapters/dds_adapter.cpp
src/adapters/service_adapters/websocket_adapters/websocket_adapter.h
src/adapters/service_adapters/websocket_adapters/websocket_adapter.cpp
src/adapters/interface_adapters/interface_bridge_manager.h
src/adapters/interface_adapters/interface_transport_adapter.h
src/dds/dds_publisher.h
src/dds/dds_publisher.cpp
src/websocket/ws_server.h
src/websocket/ws_server.cpp
src/utils/json_helper.h
src/utils/log_util/logger.h
```

**Commit Message (for additional changes):**
```
feat: enhance Observer Pattern implementation with detailed documentation

- Add comprehensive inline code documentation
- Explain Observer Pattern usage in SensorController
- Document Bridge Pattern in BridgeManager and adapters
- Add comments explaining design decisions
- Clarify responsibility of each component
- Document data flow from gRPC to transport layers

Code improvements:
- Better separation of concerns
- Clear interface definitions
- Improved error handling
- Thread-safety considerations documented
```

---

### Branch 5: `feat/enhance-configuration-files`

**Purpose:** Improve project configuration files (proto, IDL, scripts)

**Files to Include:**

**Modified Files:**
```
proto/sensor.proto
idl/SensorData/SensorData.idl
generate_proto
generate_idl
conanfile.txt
rtps.ini
```

**Commit Message:**
```
feat: enhance configuration and generation scripts

Protocol definitions:
- Add detailed comments to sensor.proto
- Document gRPC service methods and message types
- Improve SensorData.idl with field descriptions
- Add usage examples in proto files

Generation scripts:
- Improve generate_proto with error handling
- Add generate_idl with proper path resolution
- Better error messages for development

Build configuration:
- Update conanfile.txt with all required dependencies
- Add rtps.ini configuration for DDS transport
- Ensure Docker compatibility

These changes make the project easier to:
- Understand protocol definitions
- Regenerate code safely
- Configure DDS/gRPC settings
```

---

### Branch 6: `feat/docker-improvements`

**Purpose:** Enhance Docker configuration for development and deployment

**Files to Include:**

**Modified Files:**
```
Dockerfile
docker-compose.yaml
docker/CMakeLists.txt
```

**Commit Message:**
```
feat: improve Docker configuration and build process

Dockerfile improvements:
- Multi-stage build for smaller image size
- Better caching of dependencies
- Add development vs production targets
- Include all required build tools

docker-compose.yaml enhancements:
- Add service dependencies
- Configure environment variables properly
- Add volume mounts for development
- Set up networking between services

docker/CMakeLists.txt:
- Add Docker-specific build targets
- Support containerized builds
- Ensure OpenDDS paths are correct

These changes enable:
- Faster Docker builds with layer caching
- Consistent development environment
- Easy deployment to production
```

---

### Branch 7: `feat/grpc-simulator`

**Purpose:** Add gRPC client simulator for testing

**Files to Include:**

**New Files (Untracked):**
```
simulator/client.py
simulator/sensor_pb2.py
simulator/sensor_pb2_grpc.py
```

**Commit Message:**
```
feat: add gRPC client simulator for testing

- Add Python-based gRPC client simulator
- Include generated proto files for Python
- Enable easy testing of gRPC endpoints
- Support for simulating sensor data streams

Features:
- Simulates real sensor data
- Configurable sensor types and intervals
- Easy to use for development testing
- Documented usage examples

Usage:
cd simulator
python -m venv .venv
source .venv/bin/activate
pip install grpcio grpcio-tools
python client.py

Note: .venv is gitignored for local development
```

---

## Step-by-Step Execution Plan

### Phase 1: Prepare Branches

```bash
# Current directory
cd /home/felixrdev/workspace/iot-integration-project

# Ensure we're on the right branch
git checkout feat/Observer-Pattern

# Create and switch to first branch (refactoring - if observable.cpp exists)
git checkout -b refactor/separate-header-implementation

# Stage only relevant files (check if these files exist first)
git add src/adapters/abstract_adapters/observable/observable.cpp
git add src/adapters/abstract_adapters/observable/observable.h
git add src/handlers/sensor_data_log_handler/sensor_data_log_handler.cpp
git add src/handlers/sensor_data_log_handler/sensor_data_log_handler.h
git add src/handlers/sensor_data_validator/sensor_data_validator.cpp
git add src/handlers/sensor_data_validator/sensor_data_validator.h

# Commit
git commit -m "refactor: separate header and implementation files for OOP compliance

- Create .cpp files for Observable, SensorDataLogHandler, and SensorDataValidator
- Move implementations from headers to separate compilation units
- Keep only declarations in header files
- Improves compile time and follows C++ best practices
- All concrete classes now have proper header/implementation separation"

# Push to origin
git push -u origin refactor/separate-header-implementation
```

### Phase 2: Documentation Branch

``Go back to feat/Observer-Pattern
git checkout feat/Observer-Pattern

# Create docs branch
git checkout -b docs/architecture-guide

# Stage documentation files
git add docs/ARCHITECTURE.md
git add docs/GIT_STRATEGY.md
git add docs/iot-integration-project-architecture.md
git add README.md
git add README.md  # if modified for docs

# Commit
git commit -m "docs: add comprehensive architecture and workflow documentation

- Add ARCHITECTURE.md with detailed system design explanation
- Include design pattern descriptions (Observer, Bridge, Adapter)
- Add data flow diagrams and component interactions
- Provide step-by-step guide for understanding codebase"

# Push
git push -u origin docs/architecture-guide
```

### Phase 3: Build Config Branch

```bash
# Create build config branch
git checkout feat/Observer-Pattern
git checkout -b chore/build-config-improvements

# Stage build files
git add CMakeLists.txt
git add .gitignore

# Commit
git commit -m "chore: improve CMake configuration and gitignore

- Add conditional checks for OpenDDS environment variables
- Update .gitignore for clang tools and generated files
- Prevent empty-string include directory errors"

# Push
git push -u origin chore/build-config-improvements
```

### Phase 4: Configuration Files Branch

```bash
# Create config branch
git checkout feat/Observer-Pattern
git checkout -b feat/enhance-configuration-files

# Stage config files
git add proto/sensor.proto
git add idl/SensorData/SensorData.idl
git add generate_proto
git add generate_idl
git add conanfile.txt
git add rtps.ini

# Commit
git commit -m "feat: enhance configuration and generation scripts

- Add detailed comments to proto and IDL files
- Improve generation scripts with error handling
- Update conanfile.txt dependencies"

# Push
git push -u origin feat/enhance-configuration-files
```

### Phase 5: Docker Improvements Branch

```bash
# Create Docker branch
git checkout feat/Observer-Pattern
git checkout -b feat/docker-improvements

# Stage Docker files
git add Dockerfile
git add docker-compose.yaml
git add docker/CMakeLists.txt

# Commit
git commit -m "feat: improve Docker configuration and build process

- Multi-stage Dockerfile for optimized builds
- Enhanced docker-compose with proper dependencies
- Docker-specific CMake configuration"

# Push
git push -u origin feat/docker-improvements
```

### Phase 6: Update Observer Pattern Branch

```bash
# Go back to Observer Pattern branch
git checkout feat/Observer-Pattern

# Stage remaining source code changes
git add src/app.cpp
git add src/controllers/
git add src/adapters/
git add src/dds/
git add src/websocket/
git add src/utils/

# Commit
git commit -m "feat: enhance Observer Pattern implementation with detailed documentation

- Add comprehensive inline code documentation
- Explain Observer Pattern usage in components
- Document Bridge Pattern in adapters
- Clarify responsibility of each component"

# Push
git push origin feat/Observer-Pattern
```

### Phase 7: Simulator Branch

```bash
# Create simulator branch from feat/Observer-Pattern
git checkout feat/Observer-Pattern
git checkout -b feat/grpc-simulator

# Stage simulator files
git add simulator/client.py
git add simulator/sensor_pb2.py
git add simulator/sensor_pb2_grpc.py

# Commit
git commit -m "feat: add gRPC client simulator for testing

- Add Python-based gRPC client for testing
- Include generated proto stubs
- Enable easy endpoint testing"

# Push
git push -u origin feat/grpc-simulator
```

---

## Pull Request Strategy

### PR 1: Build Configuration Improvements

**Title:** Build System & Development Environment Improvements

**Branch:** `chore/build-config-improvements` → `main`

**Description:**
```
## Overview
Improves CMake configuration and development environment setup.

## Changes
- CMakeLists.txt: Add conditional OpenDDS env var checks
- .gitignore: Add clang tools, Conan artifacts, build outputs

## Why
- Prevents build errors when environment variables not set
- Keeps repository clean from generated files
- Supports multiple development environments

## Testing
- Build succeeds with and without env vars
- Clean git status after build

## Checklist
- [ ] Code builds successfully
- [ ] No new warnings
- [ ] .gitignore tested with actual builds
```

**Reviewers:** Team Lead, DevOps

**Labels:** `chore`, `build`, `configuration`

---

### PR 2: Documentation

**Title:** Add Comprehensive Architecture Documentation

**Branch:** `docs/architecture-guide` → `main`

**Description:**
```
## Overview
Adds detailed architecture documentation for the project.

## Changes
- ARCHITECTURE.md: Complete system design guide
- Design patterns explained with diagrams
- Data flow documentation
- Component interaction guide

## Why
- Helps new developers understand the system
- Documents design decisions
- Provides reference for maintenance

## Highlights
- Observer Pattern explanation
- Bridge Pattern usage
- Adapter Pattern implementation
- Step-by-step data flow

## Checklist
- [ ] All diagrams render correctly
- [ ] Code references are accurate
- [ ] Examples are clear
```

**Reviewers:** Tech Lead, Senior Developer

**Labels:** `documentation`

---

### PR 3: Separate Header/Implementation (OOP Refactoring)

**Title:** Refactor: Separate Header and Implementation Files

**Branch:** `refactor/separate-header-implementation` → `feat/Observer-Pattern`

**Description:**
```
## Overview
Refactors C++ code to follow OOP best practices by separating header declarations from implementations.

## Changes
- Create .cpp files for Observable, SensorDataLogHandler, SensorDataValidator
- Move implementation code from headers to .cpp files
- Keep only declarations in headers

## Why
- Reduces compilation time (headers change less frequently)
- Follows C++ best practices
- Makes interfaces clearer
- Improves maintainability

## Impact
- No behavioral changes
- Better compile-time performance
- Cleaner code organization

## Testing
- [ ] All unit tests pass
- [ ] Build succeeds
- [ ] No runtime behavior changes
```

**Reviewers:** C++ Expert, Team Lead

**Labels:** `refactor`, `code-quality`

**Note:** Merge this into `feat/Observer-Pattern` first, NOT main

---

### PR 4: Configuration Files Enhancement

**Title:** Enhance Proto/IDL Definitions and Generation Scripts

**Branch:** `feat/enhance-configuration-files` → `feat/Observer-Pattern`

**Description:**
```
## Overview
Improves protocol definitions and code generation scripts.

## Changes
- Add detailed comments to sensor.proto
- Document SensorData.idl fields
- Improve generate_proto script
- Update generate_idl with error handling
- Update conanfile.txt dependencies

## Why
- Better understanding of data structures
- Safer code regeneration
- Clearer protocol contracts

## Testing
- [ ] Proto generation works
- [ ] IDL generation works
- [ ] All dependencies resolve
```

**Reviewers:** Backend Developer, DevOps

**Labels:** `enhancement`, `configuration`

**Note:** Merge to `feat/Observer-Pattern` first

---

### PR 5: Docker Configuration Improvements

**Title:** Improve Docker Build and Deployment Configuration

**Branch:** `feat/docker-improvements` → `main`

**Description:**
```
## Overview
Enhances Docker configuration for better builds and deployments.

## Changes
- Multi-stage Dockerfile
- Improved docker-compose.yaml
- Docker-specific CMake configuration

## Why
- Faster builds with better caching
- Smaller production images
- Consistent development environment

## Testing
- [ ] Docker build succeeds
- [ ] docker-compose up works
- [ ] All services start correctly
```

**Reviewers:** DevOps, Infrastructure

**Labels:** `docker`, `infrastructure`

---

### PR 6: Observer Pattern Implementation (Main Feature)

**Title:** Implement Observer Pattern for Sensor Data Processing

**Branch:** `feat/Observer-Pattern` → `main`

**Description:**
```
## Overview
Implements Observer Pattern for decoupled sensor data processing and adds comprehensive code documentation.

## Changes
- Implement Observable base class
- Add SensorDataLogHandler and SensorDataValidator observers
- Integrate Observer Pattern into SensorController
- Add comprehensive inline documentation
- Explain design patterns in code comments

## Why
- Decouples data processing from data reception
- Easy to add new observers without modifying controller
- Follows SOLID principles
- Maintainable and extensible architecture

## Features
- Observer Pattern for notifications
- Bridge Pattern for transport abstraction
- Adapter Pattern for format conversion
- Thread-safe implementation

## Dependencies
This PR includes:
- refactor/separate-header-implementation (already merged)
- feat/enhance-configuration-files (already merged)

## Testing
- [ ] gRPC requests processed correctly
- [ ] Observers notified properly
- [ ] Data broadcast to WebSocket and DDS
- [ ] No memory leaks
- [ ] Thread-safe under load

## Performance
- Minimal overhead from observer notifications
- Async broadcasting to transports
- No blocking in observer callbacks
```

**Reviewers:** Tech Lead, Senior Developers, QA

**Labels:** `feature`, `observer-pattern`, `architecture`

---

## Merge Order

```
1. chore/build-config-improvements → main
   └─ Independent, can merge anytime

2. docs/architecture-guide → main
   └─ Independent, can merge anytime

3. feat/grpc-simulator → main
   └─ Independent, can merge anytime

4. refactor/separate-header-implementation → feat/Observer-Pattern
   └─ Only if separate .cpp files exist

5. feat/enhance-configuration-files → feat/Observer-Pattern
   └─ Must merge to feature branch first

6. feat/Observer-Pattern → main
   └─ After #4 and #5 are merged in (if applicable)

7. feat/docker-improvements → main
   └─ Can merge independently or after Observer Pattern
```

---

## Git Command Reference

### Quick Commands

**Check what branch you're on:**
```bash
git branch
```

**Create and switch to new branch:**
```bash
git checkout -b branch-name
```

**Stage specific files:**
```bash
git add file1 file2 file3
```

**Stage files by pattern:**
```bash
git add src/handlers/**/*.cpp
```

**Commit with message:**
```bash
git commit -m "type: description"
```

**Push new branch to remote:**
```bash
git push -u origin branch-name
```

**Switch between branches:**
```bash
git checkout branch-name
```

**See what changed:**
```bash
git status
git diff
git diff --stat
```

**Undo unstaged changes:**
```bash
git restore filename
git restore .  # all files
```

**Stash changes temporarily:**
```bash
git stash push -m "description"
git stash list
git stash pop
git stash apply
```

---

## Notes

1. **Always pull before creating new branch:**
   ```bash
   git checkout main
   git pull origin main
   git checkout -b new-branch
   ```

2. **Test before pushing:**
   - Build the code
   - Run tests
   - Check for warnings

3. **Write clear commit messages:**
   - Use conventional commit format
   - Explain WHY, not just WHAT

4. **Keep PRs focused:**
   - One concern per PR
   - Easier to review
   - Faster to merge

5. **Tag reviewers appropriately:**
   - Build/infra changes → DevOps
   - Code changes → Team Lead
   - Docs → Tech Writer or Senior Dev

---

## Quick Start Commands

### Option 1: Create All Branches at Once (Recommended)

```bash
cd /home/felixrdev/workspace/iot-integration-project

# 1. Documentation Branch (can merge to main independently)
git checkout feat/Observer-Pattern
git checkout -b docs/architecture-guide
git add docs/
git add README.md
git commit -m "docs: add comprehensive architecture and workflow documentation"
git pSimulator Branch (can merge to main independently)
git checkout feat/Observer-Pattern
git checkout -b feat/grpc-simulator
git add simulator/client.py simulator/sensor_pb2.py simulator/sensor_pb2_grpc.py
git commit -m "feat: add gRPC client simulator for testing"
git push -u origin feat/grpc-simulator

# 6. Update Observer Pattern Branch (merge to main after others)
git checkout feat/Observer-Pattern
git add src/
git commit -m "feat: enhance Observer Pattern implementation with detailed documentation"
git push origin feat/Observer-Pattern
```

### Option 2: Step by Step (If unsure)

Follow the detailed phase-by-phase instructions below.

---

## Summary

Total Branches to Create: 5 new branches

1. `docs/architecture-guide` - Documentation (merge to main)
2. `chore/build-config-improvements` - Build system (merge to main)
3. `feat/enhance-configuration-files` - Config files (merge to feat/Observer-Pattern)
4. `feat/docker-improvements` - Docker (merge to main)
5. `feat/grpc-simulator` - Testing tool (merge to main)

Plus update existing:
6 5. Update Observer Pattern Branch (merge to main after others)
git checkout feat/Observer-Pattern
git add src/
git commit -m "feat: enhance Observer Pattern implementation with detailed documentation"
git push origin feat/Observer-Pattern
```

### Option 2: Step by Step (If unsure)

Follow the detailed phase-by-phase instructions below.

---

## Summary

Total Branches to Create: 4 new branches

1. `docs/architecture-guide` - Documentation (merge to main)
2. `chore/build-config-improvements` - Build system (merge to main)
3. `feat/enhance-configuration-files` - Config files (merge to feat/Observer-Pattern)
4. `feat/docker-improvements` - Docker (merge to main)

Plus update existing:
5. `feat/Observer-Pattern` - Main feature (merge to main last)

**Note:** The `refactor/separate-header-implementation` branch is only needed if you have created separate .cpp files for Observable, SensorDataLogHandler, and SensorDataValidator.

All branches will eventually merge to `main` through proper PR process.
