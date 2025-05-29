#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>
#include <limits>
#include <algorithm>
#include <set>
#include <chrono>
#include <random>


// --- ESTRUCTURAS DE DATOS ---
struct Customer {
    int id;
    double x, y;
};

struct Vehicle {
    int id;
    int capacity;
};

class Route {
public:
    int vehicleId;
    std::vector<Customer> customers;
    double totalDistance;
    int currentLoad;

    Route(int vId = -1) : vehicleId(vId), totalDistance(0.0), currentLoad(0) {}
};

class ProblemData {
public:
    Customer depot;
    std::vector<Customer> customers;
    std::vector<Vehicle> vehicles;
    std::vector<std::vector<double>> distanceMatrix;

    ProblemData() : depot({0, 0.0, 0.0}) {}

    void loadData(const std::string& coordsFilePath, const std::string& distMatrixFilePath, int numVehicles, int vehicleCapacity) {
        // Cargar coordenadas desde Coord.txt
        std::ifstream coordsFile(coordsFilePath);
        if (!coordsFile.is_open()) {
            throw std::runtime_error("No se pudo abrir Coord.txt");
        }

        std::string line;
        int currentId = 0;

        // Leer la primera línea como el depósito
        if (std::getline(coordsFile, line)) {
            std::stringstream ss(line);
            double x, y;
            if (ss >> x >> y) {
                depot = {currentId++, x, y}; // ID del depósito es 0
            } else {
                throw std::runtime_error("Formato incorrecto en la primera línea de Coord.txt (depósito).");
            }
        } else {
            throw std::runtime_error("Coord.txt está vacío, no se pudo leer el depósito.");
        }

        // Leer las líneas restantes como clientes
        while (std::getline(coordsFile, line)) {
            std::stringstream ss(line);
            double x, y;
            if (ss >> x >> y) {
                customers.push_back({currentId++, x, y}); // Los clientes tendrán IDs secuenciales a partir de 1
            } else {
                std::cerr << "Advertencia: Línea mal formateada en Coord.txt: " << line << std::endl;
            }
        }
        coordsFile.close();

        // Verificar si se cargó el número correcto de nodos
        if (customers.size() != 199) {
            std::cerr << "Advertencia: Se esperaban 199 clientes, pero se encontraron " << customers.size() << " en Coord.txt." << std::endl;
        }
        if (depot.id != 0) { // Asegurarse de que el ID del depósito sea 0
             std::cerr << "Advertencia: El ID del depósito no es 0. Se asignará 0." << std::endl;
             depot.id = 0;
        }

        // Cargar matriz de distancias desde Dist.txt
        std::ifstream distFile(distMatrixFilePath);
        if (!distFile.is_open()) {
            throw std::runtime_error("No se pudo abrir Dist.txt");
        }

        distanceMatrix.clear(); // Limpiar cualquier dato previo
        int rowCount = 0;
        while (std::getline(distFile, line)) {
            std::stringstream ss(line);
            std::vector<double> row;
            double dist_val;
            while (ss >> dist_val) {
                row.push_back(dist_val);
            }
            distanceMatrix.push_back(row);
            rowCount++;
        }
        distFile.close();

        // Verificar dimensiones de la matriz de distancias
        int expectedNodes = 1 + customers.size(); // Depósito + Clientes
        if (distanceMatrix.size() != expectedNodes || (expectedNodes > 0 && distanceMatrix[0].size() != expectedNodes)) {
            std::cerr << "Advertencia: La matriz de distancias no tiene las dimensiones esperadas ("
                      << expectedNodes << "x" << expectedNodes << "). "
                      << "Dimensiones encontradas: " << distanceMatrix.size() << "x"
                      << (distanceMatrix.empty() ? 0 : distanceMatrix[0].size()) << std::endl;
            // Opcionalmente, se podría intentar redimensionar/reconstruir la matriz aquí,
            // pero es mejor asegurar la consistencia de los datos de entrada.
        }

        // Inicializar vehículos
        vehicles.clear();
        for (int i = 0; i < numVehicles; ++i) {
            vehicles.push_back({i, vehicleCapacity});
        }
    }

    double getDistance(int fromId, int toId) const {
        return distanceMatrix[fromId][toId];
    }
};

class Solution {
public:
    std::vector<Route> routes;
    double totalCost = 0.0;

    void calculateTotalCost(const ProblemData& data) {
        totalCost = 0.0;
        for (auto& r : routes) {
            r.totalDistance = 0.0;
            if (r.customers.empty()) continue;
            r.totalDistance += data.getDistance(data.depot.id, r.customers.front().id);
            for (size_t i = 0; i < r.customers.size() - 1; ++i)
                r.totalDistance += data.getDistance(r.customers[i].id, r.customers[i + 1].id);
            r.totalDistance += data.getDistance(r.customers.back().id, data.depot.id);
            totalCost += r.totalDistance;
        }
    }

    bool isValid(const ProblemData& data) const {
        std::set<int> visited;
        for (const auto& r : routes) {
            if (r.vehicleId < 0 || r.vehicleId >= data.vehicles.size()) return false;
            if (r.currentLoad > data.vehicles[r.vehicleId].capacity) return false;
            for (const auto& c : r.customers) visited.insert(c.id);
        }
        return visited.size() == data.customers.size();
    }
    void optimizeRoutes2Opt(const ProblemData& data) {
    for (auto& route : routes) {
        bool improved = true;
        int n = route.customers.size();
        if (n < 4) continue; // no vale la pena aplicar 2-opt si hay pocos clientes

        while (improved) {
            improved = false;
            for (int i = 0; i < n - 1; ++i) {
                for (int j = i + 2; j < n; ++j) {
                    if (j + 1 >= n) continue; // evita out-of-bounds
                    double before = data.getDistance(route.customers[i].id, route.customers[i + 1].id) +
                                    data.getDistance(route.customers[j].id, route.customers[j + 1].id);
                    double after = data.getDistance(route.customers[i].id, route.customers[j].id) +
                                   data.getDistance(route.customers[i + 1].id, route.customers[j + 1].id);
                    if (after < before) {
                        std::reverse(route.customers.begin() + i + 1, route.customers.begin() + j + 1);
                        improved = true;
                    }
                }
            }
        }
    }
    calculateTotalCost(data);
}
};

class ClarkeWright {
public:
    ClarkeWright(const ProblemData& d) : data(d) {}
    Solution solve();

private:
    const ProblemData& data;
    struct Savings { double value; int i, j; bool operator<(const Savings& s) const { return value > s.value; } };
    std::pair<int, int> findCustomer(int id, const std::vector<Route>& r) const;
};

std::pair<int, int> ClarkeWright::findCustomer(int id, const std::vector<Route>& routes) const {
    for (size_t i = 0; i < routes.size(); ++i)
        for (size_t j = 0; j < routes[i].customers.size(); ++j)
            if (routes[i].customers[j].id == id) return {(int)i, (int)j};
    return {-1, -1};
}

Solution ClarkeWright::solve() {
    std::vector<Route> routes;
    for (const auto& c : data.customers) {
        Route r; r.vehicleId = c.id % data.vehicles.size();
        r.customers.push_back(c); r.currentLoad = 1; routes.push_back(r);
    }
    std::vector<Savings> savings;
    for (size_t i = 0; i < data.customers.size(); ++i)
        for (size_t j = i + 1; j < data.customers.size(); ++j) {
            double s = data.getDistance(0, data.customers[i].id) +
                       data.getDistance(0, data.customers[j].id) -
                       data.getDistance(data.customers[i].id, data.customers[j].id);
            savings.push_back({s, (int)i, (int)j});
        }
    std::sort(savings.begin(), savings.end());

    for (const auto& s : savings) {
        int id1 = data.customers[s.i].id, id2 = data.customers[s.j].id;
        auto [ri, pi] = findCustomer(id1, routes);
        auto [rj, pj] = findCustomer(id2, routes);
        if (ri == rj || ri == -1 || rj == -1) continue;
        auto& R1 = routes[ri], R2 = routes[rj];
        if ((pi != 0 && pi != R1.customers.size() - 1) ||
            (pj != 0 && pj != R2.customers.size() - 1)) continue;
        if (R1.currentLoad + R2.currentLoad > data.vehicles[R1.vehicleId].capacity) continue;
        Route merged(R1.vehicleId);
        merged.currentLoad = R1.currentLoad + R2.currentLoad;
        if (pi == R1.customers.size() - 1 && pj == 0) {
            merged.customers = R1.customers;
            merged.customers.insert(merged.customers.end(), R2.customers.begin(), R2.customers.end());
        } else if (pi == 0 && pj == R2.customers.size() - 1) {
            merged.customers = R2.customers;
            merged.customers.insert(merged.customers.end(), R1.customers.begin(), R1.customers.end());
        } else continue;
        routes.erase(routes.begin() + std::max(ri, rj));
        routes.erase(routes.begin() + std::min(ri, rj));
        routes.push_back(merged);
    }

    Solution sol; sol.routes = routes; sol.calculateTotalCost(data);
    return sol;
}

std::mt19937 initRandomEngine(bool fixed = false, unsigned int seed = 42) {
    if (fixed) return std::mt19937(seed);
    return std::mt19937(std::chrono::high_resolution_clock::now().time_since_epoch().count());
}

void exportSolutionToCSV(const Solution& sol, const ProblemData& data, const std::string& file) {
    std::ofstream f(file);
    for (const auto& r : sol.routes) {
        if (r.vehicleId < 0) continue;
        f << "# Ruta vehiculo " << r.vehicleId << "\n";
        f << data.depot.x << "," << data.depot.y << "," << data.depot.id << "\n";
        for (const auto& c : r.customers) f << c.x << "," << c.y << "," << c.id << "\n";
        f << data.depot.x << "," << data.depot.y << "," << data.depot.id << "\n";
    }
    f.close();
    std::cout << "CSV generado: " << file << std::endl;
}

int main() {
    ProblemData data;
    try { data.loadData("data/Coord.txt", "data/Dist.txt", 20, 12); }
    catch (const std::exception& e) { std::cerr << e.what() << std::endl; return 1; }

    std::mt19937 gen = initRandomEngine(false);
    ClarkeWright cw(data);
    Solution s = cw.solve();
    s.optimizeRoutes2Opt(data);
    s.calculateTotalCost(data);

    if (!s.isValid(data)) std::cerr << "Solución inicial inválida." << std::endl;
    if (s.routes.size() > data.vehicles.size()) std::cerr << "Más rutas que vehículos." << std::endl;

    std::cout << "\nCosto total: " << s.totalCost << ", Rutas: " << s.routes.size()  << std::endl;
    std::cout << data.depot.x << "," << data.depot.y << " (Depósito)\n";
    for (size_t i = 0; i < s.routes.size(); ++i) {
        const auto& r = s.routes[i];
        std::cout << "Ruta " << i+1 << " (Vehiculo " << r.vehicleId << ", Clientes: " << r.currentLoad << "): Depot -> ";
        for (const auto& c : r.customers) std::cout << c.id << " -> ";
        std::cout << "Depot (" << r.totalDistance << ")\n";
    }

    exportSolutionToCSV(s, data, "routes_solution.csv");
    return 0;
}
