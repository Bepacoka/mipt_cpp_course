#include "os_source.h"

#include <format>
#include <stdexcept>

namespace nano_edr {

OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

void OsSource::Run(IEventHandler* agent) {
    handle_.Subscribe(&OsSource::Trampoline, agent);
    handle_.Start();

    while (true) {
        const os_status status = handle_.Wait(1000);
        if (status == OS_TIMEOUT) {
            agent->OnTick();
            continue;
        }
        if (status == OS_OK) return;

        throw std::runtime_error(std::format(
            "os_wait failed: {}", os_status_str(status)
        ));
    }
}

void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept {
    try {
        auto* agent = static_cast<IEventHandler*>(ctx);
        if (!agent || !ev) return;

        EventParts parts;
        parts.ts = std::to_string(ev->ts);
        parts.type = ev->type ? ev->type : "";
        if (ev->pid != 0) parts.pid = std::to_string(ev->pid);

        if (ev->field_count != 0 && !ev->fields) {
            throw std::invalid_argument("отсутствуют поля события");
        }

        parts.fields.reserve(ev->field_count);
        for (std::size_t i = 0; i < ev->field_count; ++i) {
            const auto& field = ev->fields[i];
            if (!field.key || !field.value) {
                throw std::invalid_argument("пустой указатель в поле события");
            }
            parts.fields.push_back({field.key, field.value});
        }

        Event event(parts);
        agent->HandleEvent(event);
    } catch (const std::exception& error) {
        //if (ctx) static_cast<Agent*>(ctx)->Warn(error.what());
        if (ctx) static_cast<IEventHandler*>(ctx)->OnBroken();
    } catch (...) {
        //if (ctx) static_cast<Agent*>(ctx)->Warn("неизвестная ошибка обработки события");
        if (ctx) static_cast<IEventHandler*>(ctx)->OnBroken();
    }
}

}  // namespace nano_edr
