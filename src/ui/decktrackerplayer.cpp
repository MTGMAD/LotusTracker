#include "decktrackerplayer.h"
#include "../macros.h"

#include <QFontDatabase>

DeckTrackerPlayer::DeckTrackerPlayer(QWidget *parent) : DeckTrackerBase(parent)
{
    applyCurrentSettings();
    // Statistics
    int statisticsFontSize = 8;
#if defined Q_OS_MAC
    statisticsFontSize += 2;
#endif
    int belerenID = QFontDatabase::addApplicationFont(":/res/fonts/Beleren-Bold.ttf");
    statisticsFont.setFamily(QFontDatabase::applicationFontFamilies(belerenID).at(0));
    statisticsFont.setPointSize(statisticsFontSize);
    statisticsPen = QPen(Qt::white);
}

DeckTrackerPlayer::~DeckTrackerPlayer()
{

}

void DeckTrackerPlayer::onPositionChanged()
{
    APP_SETTINGS->setDeckTrackerPlayerPos(uiPos);
}

void DeckTrackerPlayer::onScaleChanged()
{
    APP_SETTINGS->setDeckTrackerPlayerScale(uiScale);
}

void DeckTrackerPlayer::afterPaintEvent(QPainter &painter)
{
    int preferencesButtonSize = 16;
    int preferencesButtonMargin = 3;
    int preferencesButtonY = uiPos.y() + preferencesButtonMargin;
    QImage settingsPlus(":res/preferences.png");
    QImage settingsPlusScaled = settingsPlus.scaled(preferencesButtonSize, preferencesButtonSize,
                                            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    int settingsPlusX = uiPos.x() + uiWidth - preferencesButtonSize - preferencesButtonMargin;
    painter.drawImage(settingsPlusX, preferencesButtonY, settingsPlusScaled);
    preferencesButton = QRect(settingsPlusX*uiScale, preferencesButtonY*uiScale,
                           preferencesButtonSize*uiScale, preferencesButtonSize*uiScale);
    // Statistics
    if (!hidden && isStatisticsEnabled) {
        drawStatistics(painter);
    }
}

void DeckTrackerPlayer::applyCurrentSettings()
{
    uiPos = APP_SETTINGS->getDeckTrackerPlayerPos(uiWidth);
    uiScale = APP_SETTINGS->getDeckTrackerPlayerScale();
    isStatisticsEnabled = APP_SETTINGS->isDeckTrackerPlayerStatisticsEnabled();
}

void DeckTrackerPlayer::drawStatistics(QPainter &painter)
{
    if (deck.currentCards().size() == 0) {
        return;
    }
    // Statistics BG
    QRect coverRect(uiPos.x(), uiPos.y() + uiHeight, uiWidth, uiWidth/4);
    painter.setPen(bgPen);
    painter.setBrush(QBrush(QColor(70, 70, 70, 175)));
    painter.drawRoundedRect(coverRect, cornerRadius, cornerRadius);
    // Statistics info
    QFontMetrics statisticsMetrics(statisticsFont);
    int statisticsTextHeight = statisticsMetrics.ascent() - statisticsMetrics.descent();
    int statisticsTextMargin = 5;
    // Statistics title
    int statisticsBorderMargin = 2;
    int statisticsTitleOptions = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextDontClip;
    QString statisticsTitle = QString(tr("Draw chances:"));
    int statisticsTitleX = uiPos.x() + 8;
    int statisticsTitleY = uiPos.y() + uiHeight + statisticsTextMargin;
    drawText(painter, statisticsFont, statisticsPen, statisticsTitle, statisticsTitleOptions, false,
             statisticsTitleX, statisticsTitleY, statisticsTextHeight, uiWidth - statisticsBorderMargin);
    // Statistics text
    int statisticsTextOptions = Qt::AlignCenter | Qt::AlignVCenter | Qt::TextDontClip;
    int statisticsTextX = uiPos.x() + statisticsBorderMargin;
    int statisticsText1Y = statisticsTitleY + statisticsTextHeight + statisticsTextMargin + statisticsBorderMargin;
    float totalCards = deck.totalCards();
    float drawChanceLandCards = deck.totalCardsLand() * 100 / totalCards;
    float drawChance1xCards = deck.totalCardsOfQtd(1) > 0 ? 1 * 100 / totalCards : 0;
    float drawChance2xCards = deck.totalCardsOfQtd(2) > 0 ? 2 * 100 / totalCards : 0;
    float drawChance3xCards = deck.totalCardsOfQtd(3) > 0 ? 3 * 100 / totalCards : 0;
    float drawChance4xCards = deck.totalCardsOfQtd(4) > 0 ? 4 * 100 / totalCards : 0;
    QString statisticsText1 = QString("1x: %1%    2x: %2%    3x: %3%")
            .arg(drawChance1xCards, 0, 'g', 2)
            .arg(drawChance2xCards, 0, 'g', 2)
            .arg(drawChance3xCards, 0, 'g', 2);
    drawText(painter, statisticsFont, statisticsPen, statisticsText1, statisticsTextOptions, false,
             statisticsTextX, statisticsText1Y, statisticsTextHeight, uiWidth - statisticsBorderMargin);
    int statisticsText2Y = statisticsText1Y + statisticsTextHeight + statisticsTextMargin;
    QString statisticsText2 = QString("4x: %1%    Land: %2%")
            .arg(drawChance4xCards, 0, 'g', 2)
            .arg(drawChanceLandCards, 0, 'g', 2);
    drawText(painter, statisticsFont, statisticsPen, statisticsText2, statisticsTextOptions, false,
             statisticsTextX, statisticsText2Y, statisticsTextHeight, uiWidth - statisticsBorderMargin);
    uiHeight += coverRect.height();
}

QString DeckTrackerPlayer::onGetDeckColorIdentity()
{
    return deck.colorIdentity();
}

void DeckTrackerPlayer::loadDeck(Deck deck)
{
    this->deck = deck;
    this->deck.showOnlyRemainingCards = APP_SETTINGS->isShowOnlyRemainingCardsEnabled();
    LOGI(QString("Loading deck %1").arg(deck.name));
}

void DeckTrackerPlayer::resetDeck()
{
    deck.reset();
    update();
}

void DeckTrackerPlayer::onPlayerPutInLibraryCard(Card* card)
{
    deck.insertCard(card);
}

void DeckTrackerPlayer::onPlayerDrawCard(Card* card)
{
    if (deck.drawCard(card)) {
        blinkCard(card);
    }
}

void DeckTrackerPlayer::onPlayerDiscardCard(Card* card)
{
    if (deck.drawCard(card)) {
        blinkCard(card);
    }
}

void DeckTrackerPlayer::onPlayerDiscardFromLibraryCard(Card* card)
{
    if (deck.drawCard(card)) {
        blinkCard(card);
    }
}

void DeckTrackerPlayer::onPlayerPutOnBattlefieldCard(Card* card)
{
    if (deck.drawCard(card)) {
        blinkCard(card);
    }
}

void DeckTrackerPlayer::onShowOnlyRemainingCardsEnabled(bool enabled)
{
    deck.showOnlyRemainingCards = enabled;
    update();
}

void DeckTrackerPlayer::onStatisticsEnabled(bool enabled)
{
    isStatisticsEnabled = enabled;
    update();
}


void DeckTrackerPlayer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        return;
    }
    if (preferencesButton.contains(event->pos())) {
        return;
    }
    DeckTrackerBase::mousePressEvent(event);
}

void DeckTrackerPlayer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton && event->button() != Qt::RightButton) {
        return;
    }
    if (preferencesButton.contains(event->pos())) {
        ARENA_TRACKER->showPreferencesScreen();
        showCardOnHover = false;
        return;
    }
    DeckTrackerBase::mouseReleaseEvent(event);
}
