#include "file_source.h"

#include "event.h"
#include "parse.h"

#include <stdexcept>
#include <string>

namespace nano_edr {

FileSource::FileSource(const std::string& path) : path_(path), log_(path) {
    if (!log_) {
        throw std::runtime_error("не удалось открыть журнал: " + path_);
    }
}

void FileSource::Run(IEventHandler* agent) {
    std::string line;

    while (std::getline(log_, line)) {
        try {
            if (IsBlankOrComment(line)) {
                agent->OnSkipped();
                continue;
            }

            EventParts parts;
            if (!ParseEventParts(line, &parts)) {
                agent->OnBroken();
                //agent->Warn(std::format(
                //    "строка {} не соответствует формату", line_number
                //));
                continue;
            }
            
            Event event(parts);
            agent->HandleEvent(event);
        } catch (const std::exception& error) {
            agent->OnBroken();
            //agent->Warn(std::format("строка {}: {}", line_number, error.what()));
        } catch (...) {
            agent->OnBroken();
            //agent->Warn(std::format("строка {}: неизвестная ошибка", line_number));
        }
    }

    if (log_.bad()) {
        throw std::runtime_error("ошибка чтения журнала: " + path_);
    }
}

}  // namespace nano_edr
