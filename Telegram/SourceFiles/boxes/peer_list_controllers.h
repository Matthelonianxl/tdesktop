/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "boxes/peer_list_box.h"
#include "base/flat_set.h"
#include "base/weak_ptr.h"

// Not used for now.
//
//class MembersAddButton : public Ui::RippleButton {
//public:
//	MembersAddButton(QWidget *parent, const style::TwoIconButton &st);
//
//protected:
//	void paintEvent(QPaintEvent *e) override;
//
//	QImage prepareRippleMask() const override;
//	QPoint prepareRippleStartPosition() const override;
//
//private:
//	const style::TwoIconButton &_st;
//
//};

class PeerListRowWithLink : public PeerListRow {
public:
	using PeerListRow::PeerListRow;

	void setActionLink(const QString &action);

	void lazyInitialize(const style::PeerListItem &st) override;

private:
	void refreshActionLink();
	QSize actionSize() const override;
	QMargins actionMargins() const override;
	void paintAction(
		Painter &p,
		TimeMs ms,
		int x,
		int y,
		int outerWidth,
		bool selected,
		bool actionSelected) override;

	QString _action;
	int _actionWidth = 0;

};

class PeerListGlobalSearchController : public PeerListSearchController, private MTP::Sender {
public:
	PeerListGlobalSearchController();

	void searchQuery(const QString &query) override;
	bool isLoading() override;
	bool loadMoreRows() override {
		return false;
	}

private:
	bool searchInCache();
	void searchOnServer();
	void searchDone(const MTPcontacts_Found &result, mtpRequestId requestId);

	base::Timer _timer;
	QString _query;
	mtpRequestId _requestId = 0;
	std::map<QString, MTPcontacts_Found> _cache;
	std::map<mtpRequestId, QString> _queries;

};

class ChatsListBoxController
	: public PeerListController
	, protected base::Subscriber {
public:
	ChatsListBoxController(
		std::unique_ptr<PeerListSearchController> searchController
			= std::make_unique<PeerListGlobalSearchController>());

	void prepare() override final;
	std::unique_ptr<PeerListRow> createSearchRow(not_null<PeerData*> peer) override final;

protected:
	class Row : public PeerListRow {
	public:
		Row(not_null<History*> history);

		not_null<History*> history() const {
			return _history;
		}

	private:
		not_null<History*> _history;

	};
	virtual std::unique_ptr<Row> createRow(not_null<History*> history) = 0;
	virtual void prepareViewHook() = 0;
	virtual void updateRowHook(not_null<Row*> row) {
	}
	virtual QString emptyBoxText() const;

private:
	void rebuildRows();
	void checkForEmptyRows();
	bool appendRow(not_null<History*> history);

};

class ContactsBoxController
	: public PeerListController
	, protected base::Subscriber {
public:
	ContactsBoxController(
		std::unique_ptr<PeerListSearchController> searchController
		= std::make_unique<PeerListGlobalSearchController>());

	void prepare() override final;
	std::unique_ptr<PeerListRow> createSearchRow(not_null<PeerData*> peer) override final;
	void rowClicked(not_null<PeerListRow*> row) override;

protected:
	virtual std::unique_ptr<PeerListRow> createRow(not_null<UserData*> user);
	virtual void prepareViewHook() {
	}
	virtual void updateRowHook(not_null<PeerListRow*> row) {
	}

private:
	void rebuildRows();
	void checkForEmptyRows();
	bool appendRow(not_null<UserData*> user);

};

class AddBotToGroupBoxController
	: public ChatsListBoxController
	, public base::has_weak_ptr {
public:
	static void Start(not_null<UserData*> bot);

	AddBotToGroupBoxController(not_null<UserData*> bot);

	void rowClicked(not_null<PeerListRow*> row) override;

protected:
	std::unique_ptr<Row> createRow(not_null<History*> history) override;
	void prepareViewHook() override;
	QString emptyBoxText() const override;

private:
	static bool SharingBotGame(not_null<UserData*> bot);

	bool needToCreateRow(not_null<PeerData*> peer) const;
	bool sharingBotGame() const;
	QString noResultsText() const;
	QString descriptionText() const;
	void updateLabels();

	void shareBotGame(not_null<PeerData*> chat);
	void addBotToGroup(not_null<PeerData*> chat);

	not_null<UserData*> _bot;

};

class ChooseRecipientBoxController
	: public ChatsListBoxController
	, public base::has_weak_ptr {
public:
	ChooseRecipientBoxController(
		FnMut<void(not_null<PeerData*>)> callback);

	void rowClicked(not_null<PeerListRow*> row) override;

	bool respectSavedMessagesChat() const override {
		return true;
	}

protected:
	void prepareViewHook() override;
	std::unique_ptr<Row> createRow(
		not_null<History*> history) override;

private:
	FnMut<void(not_null<PeerData*>)> _callback;

};
