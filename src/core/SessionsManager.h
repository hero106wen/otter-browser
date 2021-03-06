/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2017 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2014 Piotr Wójcik <chocimier@tlen.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#ifndef OTTER_SESSIONSMANAGER_H
#define OTTER_SESSIONSMANAGER_H

#include "SettingsManager.h"
#include "Utils.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QRect>

namespace Otter
{

enum WindowState
{
	NormalWindowState = 0,
	MaximizedWindowState,
	MinimizedWindowState
};

struct WindowHistoryEntry
{
	QString url;
	QString title;
	QPoint position;
	int zoom = SettingsManager::getOption(SettingsManager::Content_DefaultZoomOption).toInt();
};

struct WindowHistoryInformation
{
	QVector<WindowHistoryEntry> entries;
	int index = -1;
};

struct SessionWindow
{
	QRect geometry;
	QHash<int, QVariant> options;
	QVector<WindowHistoryEntry> history;
	WindowState state = ((SettingsManager::getOption(SettingsManager::Interface_NewTabOpeningActionOption).toString() == QLatin1String("maximizeTab")) ? MaximizedWindowState : NormalWindowState);
	int parentGroup = 0;
	int historyIndex = -1;
	bool isAlwaysOnTop = false;
	bool isPinned = false;

	QString getUrl() const
	{
		if (historyIndex >= 0 && historyIndex < history.count())
		{
			return history.at(historyIndex).url;
		}

		return QString();
	}

	QString getTitle() const
	{
		if (historyIndex >= 0 && historyIndex < history.count())
		{
			if (!history.at(historyIndex).title.isEmpty())
			{
				return history.at(historyIndex).title;
			}

			if (history.at(historyIndex).url == QLatin1String("about:start") || (SettingsManager::getOption(SettingsManager::StartPage_EnableStartPageOption).toBool() && Utils::isUrlEmpty(history.at(historyIndex).url)))
			{
				return QCoreApplication::translate("main", "Start Page");
			}
		}

		return QCoreApplication::translate("main", "(Untitled)");
	}

	int getZoom() const
	{
		if (historyIndex >= 0 && historyIndex < history.count())
		{
			return history.at(historyIndex).zoom;
		}

		return SettingsManager::getOption(SettingsManager::Content_DefaultZoomOption).toInt();
	}
};

struct SessionMainWindow
{
	QVector<SessionWindow> windows;
	QByteArray geometry;
	int index = -1;
};

struct SessionInformation
{
	QString path;
	QString title;
	QVector<SessionMainWindow> windows;
	int index = -1;
	bool isClean = true;
};

struct ClosedWindow
{
	SessionWindow window;
	quint64 nextWindow = 0;
	quint64 previousWindow = 0;
	bool isPrivate = false;
};

class MainWindow;
class SessionModel;

class SessionsManager final : public QObject
{
	Q_OBJECT

public:
	enum OpenHint
	{
		DefaultOpen = 0,
		PrivateOpen = 1,
		CurrentTabOpen = 2,
		NewTabOpen = 4,
		NewWindowOpen = 8,
		BackgroundOpen = 16,
		EndOpen = 32
	};

	Q_DECLARE_FLAGS(OpenHints, OpenHint)

	static void createInstance(const QString &profilePath, const QString &cachePath, bool isPrivate = false, bool isReadOnly = false, QObject *parent = nullptr);
	static void clearClosedWindows();
	static void storeClosedWindow(MainWindow *window);
	static void markSessionModified();
	static void removeStoredUrl(const QString &url);
	static SessionsManager* getInstance();
	static SessionModel* getModel();
	static QString getCurrentSession();
	static QString getCachePath();
	static QString getProfilePath();
	static QString getReadableDataPath(const QString &path, bool forceBundled = false);
	static QString getWritableDataPath(const QString &path);
	static QString getSessionPath(const QString &path, bool isBound = false);
	static SessionInformation getSession(const QString &path);
	static QStringList getClosedWindows();
	static QStringList getSessions();
	static SessionsManager::OpenHints calculateOpenHints(OpenHints hints = DefaultOpen, Qt::MouseButton button = Qt::LeftButton, int modifiers = -1);
	static SessionsManager::OpenHints calculateOpenHints(const QVariantMap &parameters);
	static bool restoreClosedWindow(int index = -1);
	static bool restoreSession(const SessionInformation &session, MainWindow *window = nullptr, bool isPrivate = false);
	static bool saveSession(const QString &path = QString(), const QString &title = QString(), MainWindow *window = nullptr, bool isClean = true);
	static bool saveSession(const SessionInformation &session);
	static bool deleteSession(const QString &path = QString());
	static bool isPrivate();
	static bool isReadOnly();
	static bool hasUrl(const QUrl &url, bool activate = false);

protected:
	explicit SessionsManager(QObject *parent = nullptr);

	void timerEvent(QTimerEvent *event) override;
	void scheduleSave();

private:
	int m_saveTimer;

	static SessionsManager *m_instance;
	static SessionModel *m_model;
	static QString m_sessionPath;
	static QString m_sessionTitle;
	static QString m_cachePath;
	static QString m_profilePath;
	static QVector<SessionMainWindow> m_closedWindows;
	static bool m_isDirty;
	static bool m_isPrivate;
	static bool m_isReadOnly;

signals:
	void closedWindowsChanged();
	void requestedRemoveStoredUrl(const QString &url);
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Otter::SessionsManager::OpenHints)

#endif
