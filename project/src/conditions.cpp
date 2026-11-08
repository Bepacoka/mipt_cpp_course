#include "conditions.h"

#include "fields.h"
#include "text.h"

namespace nano_edr {

bool FieldEquals::Matches(const Event& event) const {
    const auto* field = FindField(event, key_);
    return field && TextEquals(*field, value_);
}

bool FieldContains::Matches(const Event& event) const {
    const auto* field = FindField(event, key_);
    return field && TextContains(*field, needle_);
}

bool FieldEndsWith::Matches(const Event& event) const {
    const auto* field = FindField(event, key_);
    return field && TextEndsWith(*field, suffix_);
}

bool FieldInList::Matches(const Event& event) const {
    const auto* field = FindField(event, key_);
    if (!field) return false;

    for (const auto& value : values_) {
        if (TextEquals(*field, value)) return true;
    }
    return false;
}

bool EventTypeIs::Matches(const Event& event) const {
    return std::ranges::find(types_, event.event_type()) != types_.end();
}

bool AnyOf::Matches(const Event& event) const {
    for (const auto& el: parts_) {
        if (el->Matches(event)) return true;
    }
    return false;
}

bool AllOf::Matches(const Event& event) const {
    for (const auto& el: parts_) {
        if (!el->Matches(event)) return false;
    }
    return true;
}

void AnyOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}

void AllOf::Add(std::unique_ptr<ICondition> condition) {
    parts_.push_back(std::move(condition));
}

}  // namespace nano_edr
