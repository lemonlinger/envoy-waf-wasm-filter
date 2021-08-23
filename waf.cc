#include <string>
#include <unordered_map>

#include "google/protobuf/util/json_util.h"
#include "modsecurity/modsecurity.h"
#include "modsecurity/transaction.h"
#include "modsecurity/rules_set.h"
#include "proxy_wasm_intrinsics.h"
#include "waf.pb.h"

using modsecurity::ModSecurity;
using modsecurity::RulesSet;
using modsecurity::Transaction;

class WafRootContext: public RootContext {
    public:
        explicit WafRootContext(uint32_t id, std::string_view root_id): RootContext(id, root_id), modsec_(new ModSecurity()), rules_(new RulesSet()) {}
        bool onStart(size_t configuration_size) override;
        virtual bool onConfigure(size_t) override;

        const Transaction* getTransaction() const { return transaction_; }

    private:
        ModSecurity *modsec_;
        RulesSet *rules_;
        Transaction *transaction_;
};

bool WafRootContext::onConfigure(size_t conf_size) {
    auto conf = getBufferBytes(WasmBufferType::PluginConfiguration, 0, conf_size);
    Config config;

    google::protobuf::util::JsonParseOptions options;
    options.case_insensitive_enum_parsing = true;
    options.ignore_unknown_fields = false;

    if (!conf) {
        LOG_INFO("received null config - waf filter will be disabled");
        return true;
    }

    if (conf->data() == nullptr) {
        LOG_INFO("received null config data - waf filter will be disabled");
        return true;
    }

    std::string confStr = conf->toString();

    const auto strict_status = google::protobuf::util::JsonStringToMessage(confStr, &config, options);
    if (!strict_status.ok()) {
        LOG_ERROR("failed parsing config:" + confStr + "\n error:" + strict_status.ToString());
        return false;
    }

    if (config.disable()) {
        delete transaction_;
        transaction_ = NULL;
        return true;
    }

    for (int i = 0;i < config.rules_size(); i++) {
        // rules_->load(config.rules(i).c_str(), "");
    }
    transaction_ = new Transaction(modsec_, rules_, NULL);
    return true;
}

class WafContext : public Context {
    public:
        explicit WafContext(uint32_t id, RootContext* root) : Context(id, root), root_(static_cast<const WafRootContext*>(static_cast<const void*>(root)))  {}
        FilterHeadersStatus onRequestHeaders(uint32_t headers, bool end_of_stream) override;
        void onDone() override;

    private:
        const WafRootContext *root_;
};
static RegisterContextFactory register_WafContext(CONTEXT_FACTORY(WafContext), ROOT_FACTORY(WafRootContext), "waf");

FilterHeadersStatus WafContext::onRequestHeaders(uint32_t headers, bool end_of_stream) {
    logInfo(std::string("onRequestHeaders ") + std::to_string(id()));
    logInfo(std::string("headers ") + std::to_string(headers));
    logInfo(std::string("end_of_stream ") + std::to_string(end_of_stream));
    auto path = getRequestHeader(":path");
    logInfo(std::string("header path ") + std::string(path->view()));
    return FilterHeadersStatus::Continue;            
}

void WafContext::onDone() { logInfo("onDone " + std::to_string(id()));  }
