#include <iostream>
#include <fstream>
#include <vector>
#include <cstdint>
#include <iomanip>
#include <stdexcept>

// ======================= КЛАСС BINARY PARSER =======================

class BinaryParser {
public:
    BinaryParser(const uint8_t* data, size_t size, bool bigEndian = true)
        : data_(data), size_(size), pos_(0), bigEndian_(bigEndian) {}
    
    bool available(size_t n) const { return pos_ + n <= size_; }

    void skip(size_t n) { 
        if (!available(n)) throw std::out_of_range("Попытка пропустить за пределы данных");
        pos_ += n; 
    }

    size_t position() const { return pos_; }

    bool eof() const { return pos_ >= size_; }
    
    uint8_t readU8() {
        if (!available(1)) throw std::out_of_range("Недостаточно данных для чтения uint8_t");
        return data_[pos_++];
    }
    
    uint16_t readU16() {
        if (!available(2)) throw std::out_of_range("Недостаточно данных для чтения uint16_t");
        uint16_t val;
        if (bigEndian_) {
            val = (static_cast<uint16_t>(data_[pos_]) << 8) | data_[pos_ + 1];
        } else {
            val = (static_cast<uint16_t>(data_[pos_ + 1]) << 8) | data_[pos_];
        }
        pos_ += 2;
        return val;
    }
    
    int16_t readI16() {
        uint16_t val = readU16();
        return *reinterpret_cast<const int16_t*>(&val);
    }
    
    uint32_t readU32() {
        if (!available(4)) throw std::out_of_range("Недостаточно данных для чтения uint32_t");
        uint32_t val;
        if (bigEndian_) {
            val = (static_cast<uint32_t>(data_[pos_]) << 24) |
                  (static_cast<uint32_t>(data_[pos_ + 1]) << 16) |
                  (static_cast<uint32_t>(data_[pos_ + 2]) << 8) |
                  data_[pos_ + 3];
        } else {
            val = (static_cast<uint32_t>(data_[pos_ + 3]) << 24) |
                  (static_cast<uint32_t>(data_[pos_ + 2]) << 16) |
                  (static_cast<uint32_t>(data_[pos_ + 1]) << 8) |
                  data_[pos_];
        }
        pos_ += 4;
        return val;
    }
    
    int32_t readI32() {
        uint32_t val = readU32();
        return *reinterpret_cast<const int32_t*>(&val);
    }
    
    double readScaledI16(double scale = 0.1) { return readI16() * scale; }
    double readScaledI32(double scale = 0.1) { return readI32() * scale; }
    double readScaledU8(double scale = 0.1) { return readU8() * scale; }
    
    std::vector<uint8_t> readBytes(size_t length) {
        if (!available(length)) throw std::out_of_range("Недостаточно данных для чтения байтового массива");
        std::vector<uint8_t> bytes(data_ + pos_, data_ + pos_ + length);
        pos_ += length;
        return bytes;
    }
    
    const uint8_t* currentPtr() const { 
        return (pos_ < size_) ? data_ + pos_ : nullptr; 
    }
    
    void rewind() { pos_ = 0; }
    void seek(size_t pos) { 
        if (pos > size_) throw std::out_of_range("Позиция выходит за пределы данных");
        pos_ = pos; 
    }

private:
    const uint8_t* data_;
    size_t size_;
    size_t pos_;
    bool bigEndian_;
};

// ======================= СТРУКТУРА ДЛЯ ХРАНЕНИЯ ЦЕЛИ =======================

const uint8_t TARGET_INFO_LENGTH = 29; // Сумма байт информации по цели

struct TargetInfo {
    uint8_t number;
    double verticalDist;
    double lateralDist;
    double speedY;
    uint8_t type;
    uint8_t lane;
    double frontSpace;
    double frontTime;
    double speedX;
    uint16_t heading;
    uint8_t events;
    double radarX;
    double radarY;
    uint8_t blindMark;
    double length;
    double width;
    
    void print() const {
        std::cout << "\n--- Цель #" << static_cast<int>(number) << " ---" << std::endl;
        std::cout << "  Тип: " << static_cast<int>(type);
        switch(type) {
            case 0: std::cout << " (малый автомобиль)"; break;
            case 1: std::cout << " (пешеход)"; break;
            case 2: std::cout << " (немоторизованное ТС)"; break;
            case 3: std::cout << " (средний автомобиль)"; break;
            case 4: std::cout << " (большой автомобиль)"; break;
            default: std::cout << " (неизвестный)"; break;
        }
        std::cout << std::endl;
        
        std::cout << "  Полоса: " << static_cast<int>(lane) << std::endl;
        std::cout << std::fixed << std::setprecision(1);
        std::cout << "  Расстояние: вертикальное=" << verticalDist 
                  << " м, латеральное=" << lateralDist << " м" << std::endl;
        std::cout << "  Скорость: X=" << speedX 
                  << " м/с, Y=" << speedY << " м/с" << std::endl;
        std::cout << "  Интервал: пространственный=" << frontSpace 
                  << " м, временной=" << frontTime << " с" << std::endl;
        std::cout << "  Угол курса: " << heading << "°" << std::endl;
        std::cout << "  Габариты: длина=" << length 
                  << " м, ширина=" << width << " м" << std::endl;
        std::cout << "  Сетевая позиция: X=" << radarX 
                  << " м, Y=" << radarY << " м" << std::endl;
        std::cout << "  Метка слепой зоны: " << (blindMark ? "слепой радар" : "основной радар") << std::endl;
        
        if (events != 0) {
            std::cout << "  События: ";
            const char* eventNames[] = {
                "Беспрепятственная парковка", "Заторная парковка", 
                "Превышение скорости", "Портовая парковка",
                "Медленно движущееся ТС", "Пешеход",
                "Движение назад", "Смена направления"
            };
            
            for (int bit = 0; bit < 8; ++bit) {
                if (events & (1 << bit)) {
                    std::cout << eventNames[bit] << " ";
                }
            }
            std::cout << std::endl;
        }
    }
};

// ======================= ПАРСИНГ ОДНОЙ ЦЕЛИ =======================

TargetInfo parseTarget(BinaryParser& parser) {
    TargetInfo target;
    
    target.number = parser.readU8();
    target.verticalDist = parser.readScaledI16();
    target.lateralDist = parser.readScaledI16();
    target.speedY = parser.readScaledI16();
    target.type = parser.readU8();
    target.lane = parser.readU8();
    target.frontSpace = parser.readScaledI16();
    target.frontTime = parser.readScaledI16();
    target.speedX = parser.readScaledI16();
    target.heading = parser.readU16();
    target.events = parser.readU8();
    target.radarX = parser.readScaledI32();
    target.radarY = parser.readScaledI32();
    target.blindMark = parser.readU8();
    target.length = parser.readScaledU8();
    target.width = parser.readScaledU8();
    
    return target;
}

// ======================= ПАРСИНГ МОДУЛЯ 0x4D42 =======================

void parseTargetModuleFixed(const uint8_t* moduleStart, size_t moduleSize) {
    std::cout << "\n=== МОДУЛЬ ИНФОРМАЦИИ О ЦЕЛЯХ (0x4D42) ===" << std::endl;
    std::cout << "Размер модуля: " << moduleSize << " байт" << std::endl;
    
    // Создаем парсер для всего модуля
    BinaryParser parser(moduleStart, moduleSize, true);
    
    try {
        // 1. Проверка сигнатуры
        uint16_t signature = parser.readU16();
        if (signature != 0x4D42) {
            std::cerr << "Ошибка: ожидалась сигнатура 0x4D42, получено 0x" 
                      << std::hex << signature << std::dec << std::endl;
            return;
        }
        
        // 2. Чтение длины ВСЕГО модуля (включая сигнатуру и длину)
        uint16_t totalModuleLength = parser.readU16();
        
        std::cout << "Заявленная длина модуля (B): " << totalModuleLength << " байт" << std::endl;
        
        // Проверка согласованности
        if (totalModuleLength != moduleSize) {
            std::cerr << "Предупреждение: заявленная длина (" << totalModuleLength 
                      << ") не равна фактическому размеру (" << moduleSize << ")" << std::endl;
        }
        
        // 3. Вычисление количества целей
        // Данные модуля = totalModuleLength байт
        // Заголовок (сигнатура + длина) = 4 байта
        // Контрольная сумма = 1 байт
        // Данные целей = totalModuleLength - 5 байт
        size_t targetsDataSize = totalModuleLength - 5;
        size_t targetCount = targetsDataSize / TARGET_INFO_LENGTH;
        
        std::cout << "Данные целей: " << targetsDataSize << " байт" << std::endl;
        std::cout << "Количество целей: " << targetCount << std::endl;
        
        if (targetsDataSize % TARGET_INFO_LENGTH != 0) {
            std::cerr << "Предупреждение: данные целей не кратны " << TARGET_INFO_LENGTH << " байтам" << std::endl;
        }
        
        // 4. Парсинг каждой цели
        std::vector<TargetInfo> targets;
        targets.reserve(targetCount);
        
        for (size_t i = 0; i < targetCount; ++i) {
            try {
                targets.push_back(parseTarget(parser));
            } catch (const std::exception& e) {
                std::cerr << "Не удалось распарсить цель #" << i << ": " << e.what() << std::endl;
                break;
            }
        }
        
        // 5. Вычисление контрольной суммы
        // Создаем отдельный парсер для проверки
        BinaryParser checksumParser(moduleStart, totalModuleLength, true);
        
        // Пропускаем последний байт (это сама контрольная сумма)
        uint8_t calculatedChecksum = 0;
        for (size_t i = 0; i < totalModuleLength - 1; ++i) {
            calculatedChecksum += checksumParser.readU8();
        }
        calculatedChecksum &= 0xFF;
        
        // Читаем фактическую контрольную сумму
        checksumParser.seek(totalModuleLength - 1);
        uint8_t actualChecksum = checksumParser.readU8();
        
        std::cout << "\nКонтрольная сумма: " << std::endl;
        std::cout << "  Прочитанная: 0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(actualChecksum) << std::dec << std::endl;
        std::cout << "  Вычисленная: 0x" << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(calculatedChecksum) << std::dec << std::endl;
        
        if (actualChecksum == calculatedChecksum) {
            std::cout << "  ✓ Контрольная сумма верна" << std::endl;
        } else {
            std::cout << "  ✗ Ошибка контрольной суммы!" << std::endl;
        }
        
        // 6. Проверяем, что мы прочитали все данные до контрольной суммы
        size_t expectedPosition = totalModuleLength - 1;
        if (parser.position() != expectedPosition) {
            std::cerr << "Предупреждение: позиция парсера (" << parser.position() 
                      << ") не соответствует ожидаемой (" << expectedPosition 
                      << ") перед чтением контрольной суммы" << std::endl;
            // Пропускаем оставшиеся байты до контрольной суммы
            if (parser.position() < expectedPosition) {
                parser.skip(expectedPosition - parser.position());
            }
        }
        
        // 7. Читаем контрольную сумму из основного парсера
        uint8_t moduleChecksum = parser.readU8();
        
        // 8. Вывод информации о целях
        for (const auto& target : targets) {
            target.print();
        }
        
        std::cout << "\nОбработано целей: " << targets.size() << std::endl;
        std::cout << "Обработано байт: " << parser.position() << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка парсинга модуля: " << e.what() << std::endl;
        std::cerr << "Текущая позиция парсера: " << parser.position() << std::endl;
    }
}

// ======================= ОСНОВНАЯ ФУНКЦИЯ =======================

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Использование: " << argv[0] << " <файл.bin>" << std::endl;
        return 1;
    }
    
    try {
        // Читаем файл
        std::ifstream file(argv[1], std::ios::binary | std::ios::ate);
        size_t fileSize = file.tellg();
        file.seekg(0);
        
        std::vector<uint8_t> buffer(fileSize);
        file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
        
        std::cout << "Размер файла: " << fileSize << " байт" << std::endl;
        std::cout << "Поиск модуля 0x4D42..." << std::endl;
        
        // Ищем модуль 0x4D42
        bool found = false;
        for (size_t i = 0; i <= buffer.size() - 4; ++i) {
            if (buffer[i] == 0x4D && buffer[i+1] == 0x42) {
                std::cout << "Найден модуль 0x4D42 по смещению: " << i << std::endl;
                
                // Читаем длину ВСЕГО модуля
                uint16_t totalModuleLength = (buffer[i+2] << 8) | buffer[i+3];
                
                // Проверяем, что модуль полностью в буфере
                if (i + totalModuleLength <= buffer.size()) {
                    parseTargetModuleFixed(&buffer[i], totalModuleLength);
                    found = true;
                    break;
                } else {
                    std::cerr << "Модуль выходит за пределы файла" << std::endl;
                }
            }
        }
        
        if (!found) {
            std::cerr << "Модуль 0x4D42 не найден" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}