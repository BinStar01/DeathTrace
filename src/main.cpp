#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/utils/Keyboard.hpp>
#include <unordered_map>

using namespace geode::prelude;

// 현재 각 키의 입력 상태
static std::unordered_map<int, bool> g_keyStates;

// 사망 후 입력 감시 상태
static bool g_isWatching = false;
static bool g_hasRecorded = false;

// 사망 시각
static double g_deathTimestamp = 0.0;

// 현재 화면에 표시 중인 결과 라벨
static CCLabelBMFont* g_resultLabel = nullptr;

class $modify(DeathTracePlayLayer, PlayLayer) {

    struct Fields {
        bool m_deathTraceDetected = false;
        double m_deathTimestamp = 0.0;
        CCLabelBMFont* m_resultLabel = nullptr;
    };

    bool init(GJGameLevel* level, bool dontCreateObjects) {

    // 새로운 PlayLayer 세션 시작: 이전 세션의 포인터와 측정 상태 초기화
    g_resultLabel = nullptr;
    g_isWatching = false;
    g_hasRecorded = false;
    g_deathTimestamp = 0.0;
    g_keyStates.clear();

    log::info("DeathTrace: New session - previous state cleared!");

    if (!PlayLayer::init(level, dontCreateObjects)) {
        return false;
    }

        m_fields->m_deathTraceDetected = false;
        m_fields->m_deathTimestamp = 0.0;

        // 화면 크기 가져오기
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        // 결과 표시용 라벨 생성
        auto label = CCLabelBMFont::create(
            "+0.0ms",
            "bigFont.fnt"
        );

        if (label) {
            // 처음에는 숨김
            label->setVisible(false);

            // 우측 하단에 배치
            label->setAnchorPoint({1.0f, 0.0f});
            label->setPosition({
                winSize.width - 20.0f,
                20.0f
            });

            // 글자 크기와 색상
            label->setScale(0.5f);
            label->setColor({255, 100, 100});

            // 게임 화면에 추가
            this->addChild(label, 1000);

            m_fields->m_resultLabel = label;
            g_resultLabel = label;
            log::info(
    "DeathTrace UI INIT: label created = {}",
    static_cast<void*>(g_resultLabel)
);
        }

        log::info(
            "DeathTrace: PlayLayer initialized! Instance: {}",
            static_cast<void*>(this)
        );

        return true;
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        bool isAntiCheatSpike =
            object &&
            m_anticheatSpike &&
            object->m_uniqueID == m_anticheatSpike->m_uniqueID;

        if (isAntiCheatSpike) {
            PlayLayer::destroyPlayer(player, object);
            return;
        }

        // 사망 시각 기록
        g_deathTimestamp = geode::utils::getInputTimestamp();

        m_fields->m_deathTimestamp = g_deathTimestamp;

        // 새로운 50ms 감시 시작
        g_isWatching = true;
        g_hasRecorded = false;

        // 이전 측정 결과 숨기기
        if (g_resultLabel) {
            g_resultLabel->setVisible(false);
        }

        log::info(
    "DeathTrace UI DEATH: global label={}",
    static_cast<void*>(g_resultLabel)
);      

        log::info(
            "DeathTrace: Death timestamp = {}",
            g_deathTimestamp
        );

        log::info(
            "DeathTrace DEBUG: "
            "this={} | player={} | object={} | "
            "isPlayer1={} | isDead={} | "
            "playerDied={} | locked={}",
            static_cast<void*>(this),
            static_cast<void*>(player),
            static_cast<void*>(object),
            player == m_player1,
            player ? player->m_isDead : false,
            m_playerDied,
            m_player1 ? m_player1->m_isLocked : false
        );

        PlayLayer::destroyPlayer(player, object);
    }

    // 재시작 시 결과 숨기기
    void resetLevel() {
    log::info("DeathTrace DEBUG: resetLevel called!");

    g_isWatching = false;
    g_hasRecorded = false;

    if (g_resultLabel) {
        g_resultLabel->setVisible(false);
    }

    // UI 표시 상태 확인
    log::info(
        "DeathTrace UI DEBUG: label={} | visible={}",
        static_cast<void*>(g_resultLabel),
        g_resultLabel ? g_resultLabel->isVisible() : false
    );

    PlayLayer::resetLevel();
}
    // 게임 화면에 다시 진입할 때 라벨 포인터 복구
    void onEnter() {
        PlayLayer::onEnter();

        if (m_fields->m_resultLabel) {
            g_resultLabel = m_fields->m_resultLabel;

            log::info(
                "DeathTrace UI ENTER: restored label={}",
                static_cast<void*>(g_resultLabel)
            );
        }
        else {
            log::warn(
                "DeathTrace UI ENTER: local label is NULL!"
            );
        }
    }
    
        // 게임 화면 종료 시 처리
    void onExit() {
        log::warn(
            "DeathTrace UI EXIT: this={} | global={} | local={}",
            static_cast<void*>(this),
            static_cast<void*>(g_resultLabel),
            static_cast<void*>(m_fields->m_resultLabel)
        );

        // 바닐라 Pause 메뉴에서는 PlayLayer와 자식 라벨이 유지되므로
        // 전역 라벨 포인터를 해제하지 않는다.

        PlayLayer::onExit();
    }
};

$on_mod(Loaded) {
    log::info("DeathTrace BUILD TEST 2026-11-26-A");

    static auto keyboardListener = KeyboardInputEvent().listen(
        [](KeyboardInputData& data) {
            const char* action = "Unknown";

            switch (data.action) {
                case KeyboardInputData::Action::Press:
                    action = "Press";
                    break;

                case KeyboardInputData::Action::Release:
                    action = "Release";
                    break;

                case KeyboardInputData::Action::Repeat:
                    action = "Repeat";
                    break;
            }

            // Repeat 이벤트는 상태 변화가 아니므로 무시
            if (data.action == KeyboardInputData::Action::Repeat) {
                return;
            }

            int key = static_cast<int>(data.key);

            bool newState =
                data.action == KeyboardInputData::Action::Press;

            bool oldState = g_keyStates[key];

            // 실제 상태 변화가 없다면 무시
            if (oldState == newState) {
                return;
            }

            // 현재 키 상태 갱신
            g_keyStates[key] = newState;

            // 감시 중이 아니거나 이미 기록했다면 종료
            if (!g_isWatching || g_hasRecorded) {
                return;
            }

            // 사망 시각으로부터 경과 시간 계산
            double elapsedMs =
                (data.timestamp - g_deathTimestamp) * 1000.0;

            // 50ms를 초과했다면 감시 종료
            if (elapsedMs > 50.0) {
                g_isWatching = false;
                return;
            }

            // 사망 시각보다 이전에 발생한 이벤트는 무시
            if (elapsedMs < 0.0) {
                return;
            }

            // 최초 입력 상태 변화 기록
            g_hasRecorded = true;
            
            log::info(
    "DeathTrace UI CHECK: label pointer = {}",
    static_cast<void*>(g_resultLabel)
);

            // 측정 결과를 화면에 표시
            if (g_resultLabel) {
    auto resultText = fmt::format(
        "+{:.1f}ms",
        elapsedMs
    );

    g_resultLabel->setString(resultText.c_str());
    g_resultLabel->setVisible(true);
    log::info(
    "DeathTrace UI UPDATE: {}",
    resultText
);

    log::info(
        "DeathTrace UI CHECK: text={} | visible={}",
        resultText,
        g_resultLabel->isVisible()
    );
}
else {
    log::warn("DeathTrace UI CHECK: resultLabel is NULL!");
}

            log::info(
                "DeathTrace FIRST INPUT: "
                "key={} | action={} | elapsed=+{:.3f}ms",
                key,
                action,
                elapsedMs
            );

            g_isWatching = false;
        }
    );
}