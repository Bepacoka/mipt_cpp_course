#include "agent_rules.h"
#include "combinators.h"
#include "conditions.h"
#include "make_rule.h"
#include "rule_engine.h"

namespace nano_edr {

RuleEngine BuildAgentRules() {
    RuleEngine rules;
    auto script_host = std::make_unique<MatchRule>("script_host_from_temp", Severity::kHigh);
    
    script_host->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));

    auto image_ends_with = std::make_unique<AnyOf>();
    image_ends_with->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    image_ends_with->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    script_host->AddCondition(std::move(image_ends_with));

    auto cmdline_contains_tmp = std::make_unique<AnyOf>();
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmdline_contains_tmp->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    script_host->AddCondition(std::move(cmdline_contains_tmp));
    script_host->AddAction("kill_process");
    rules.AddRule(std::move(script_host));

    auto lolbin = std::make_unique<MatchRule>("lolbin_download", Severity::kHigh);
    lolbin->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    auto image_ends_with_lolbin = std::make_unique<AnyOf>();
    image_ends_with_lolbin->Add(std::make_unique<FieldEndsWith>("image", "certutil.exe"));
    image_ends_with_lolbin->Add(std::make_unique<FieldEndsWith>("image", "bitsadmin.exe"));
    lolbin->AddCondition(std::move(image_ends_with_lolbin));

    auto cmdline_contains_lolbin = std::make_unique<AnyOf>();
    cmdline_contains_lolbin->Add(std::make_unique<FieldContains>("cmdline", "urlcache"));
    cmdline_contains_lolbin->Add(std::make_unique<FieldContains>("cmdline", "transfer"));
    cmdline_contains_lolbin->Add(std::make_unique<FieldContains>("cmdline", "http:"));
    cmdline_contains_lolbin->Add(std::make_unique<FieldContains>("cmdline", "https:"));
    lolbin->AddCondition(std::move(cmdline_contains_lolbin));
    lolbin->AddAction("kill_process");
    rules.AddRule(std::move(lolbin));

    auto powershell = std::make_unique<MatchRule>("hidden_powershell", Severity::kMedium);
    powershell->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    auto image_ends_with_pwsh = std::make_unique<AnyOf>();
    image_ends_with_pwsh->Add(std::make_unique<FieldEndsWith>("image", "powershell.exe"));
    image_ends_with_pwsh->Add(std::make_unique<FieldEndsWith>("image", "pwsh.exe"));
    powershell->AddCondition(std::move(image_ends_with_pwsh));

    auto cmdline_contains_pwsh = std::make_unique<AnyOf>();
    cmdline_contains_pwsh->Add(std::make_unique<FieldContains>("cmdline", "-w hidden"));
    cmdline_contains_pwsh->Add(std::make_unique<FieldContains>("cmdline", "-windowstyle hidden"));
    cmdline_contains_pwsh->Add(std::make_unique<FieldContains>("cmdline", "-enc"));
    cmdline_contains_pwsh->Add(std::make_unique<FieldContains>("cmdline", "-encodedcommand"));
    powershell->AddCondition(std::move(cmdline_contains_pwsh));
    powershell->AddAction("kill_process");
    rules.AddRule(std::move(powershell));

    auto autostart_write = std::make_unique<MatchRule>("autostart_write", Severity::kHigh);
    autostart_write->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileMove}));
    auto path_contains_autostart = std::make_unique<AnyOf>();
    path_contains_autostart->Add(std::make_unique<FieldContains>("path", "\\start menu\\programs\\startup\\"));
    path_contains_autostart->Add(std::make_unique<FieldContains>("path", "/start menu/programs/startup/"));
    path_contains_autostart->Add(std::make_unique<FieldContains>("to", "\\start menu\\programs\\startup\\"));
    path_contains_autostart->Add(std::make_unique<FieldContains>("to", "/start menu/programs/startup/"));
    autostart_write->AddCondition(std::move(path_contains_autostart));
    autostart_write->AddAction("quarantine_file");
    rules.AddRule(std::move(autostart_write));
    
    auto ransom_extension = std::make_unique<MatchRule>("ransom_extension", Severity::kCritical);
    ransom_extension->AddCondition(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kFileCreate, EventType::kFileMove, EventType::kFileWrite}));
    auto path_contains_ransom = std::make_unique<AnyOf>();
    path_contains_ransom->Add(std::make_unique<FieldEndsWith>("path", ".locked"));
    path_contains_ransom->Add(std::make_unique<FieldEndsWith>("to", ".locked"));
    ransom_extension->AddCondition(std::move(path_contains_ransom));
    ransom_extension->AddAction("quarantine_file");
    rules.AddRule(std::move(ransom_extension));

    auto beacon = std::make_unique<SequenceRule>("script_host_beacon", Severity::kCritical, 10000);
    auto beacon_allof = std::make_unique<AllOf>();
    beacon_allof->Add(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kProcessStart}));
    auto beacon_anyof = std::make_unique<AnyOf>();
    beacon_anyof->Add(std::make_unique<FieldEndsWith>("image", "wscript.exe"));
    beacon_anyof->Add(std::make_unique<FieldEndsWith>("image", "cscript.exe"));
    beacon_allof->Add(std::move(beacon_anyof));

    auto cmdline_contains_tmp_b = std::make_unique<AnyOf>();
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "\\appdata\\local\\temp\\"));
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "/appdata/local/temp/"));
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "\\windows\\temp\\"));
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "/windows/temp/"));
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "%temp%"));
    cmdline_contains_tmp_b->Add(std::make_unique<FieldContains>("cmdline", "%tmp%"));
    beacon_allof->Add(std::move(cmdline_contains_tmp_b));
    beacon->SetFirst(std::move(beacon_allof));
    beacon->SetSecond(std::make_unique<EventTypeIs>(std::vector<EventType>{EventType::kNetConnect}));
    beacon->AddAction("kill_process");
    rules.AddRule(std::move(beacon));

    auto ransomware_mass_write = MakeRule<ThresholdRule<128>>("ransomware_mass_write", Severity::kCritical, 20, 10000, "pid");
    ransomware_mass_write->SetCondition(MakeCondition(AllOfStatic(
                                            EventTypeIs({EventType::kFileWrite, EventType::kFileMove, EventType::kFileCreate}),
                                            AnyOfStatic(FieldEndsWith("path", ".locked"), FieldEndsWith("to", ".locked"))
                                        )));
    ransomware_mass_write->AddAction("kill_process");
    ransomware_mass_write->AddAction("quarantine_file");
    rules.AddRule(std::move(ransomware_mass_write));
    return rules;
}

}  // namespace nano_edr
