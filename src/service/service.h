#ifndef QTSERVICE_SERVICE_H
#define QTSERVICE_SERVICE_H

#include <functional>

#include <QtCore/qobject.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qdir.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qvector.h>
#include <QtCore/qhash.h>
#include <QtCore/qvariant.h>

#include "QtService/qtservice_global.h"
#include "QtService/qtservice_helpertypes.h"

namespace QtService {

class Terminal;
class ServiceBackend;
class ServicePrivate;
class Q_SERVICE_EXPORT Service : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString backend READ backend CONSTANT)
	Q_PROPERTY(QDir runtimeDir READ runtimeDir CONSTANT)
	Q_PROPERTY(bool terminalActive READ isTerminalActive WRITE setTerminalActive NOTIFY terminalActiveChanged)
	Q_PROPERTY(bool globalTerminal READ globalTerminal WRITE setGlobalTerminal NOTIFY globalTerminalChanged)

public:
	enum CommandMode {
		Synchronous,
		Asynchronous
	};
	Q_ENUM(CommandMode)

	enum TerminalMode {
		ReadOnly,
		WriteOnly,
		ReadWritePassive,
		ReadWriteActive
	};
	Q_ENUM(TerminalMode)

	explicit Service(int &argc, char **argv, int = QCoreApplication::ApplicationFlags);
	~Service() override;

	int exec();

	static Service *instance();

	QList<int> getSockets(const QByteArray &socketName);
	int getSocket();

	QString backend() const;
	QDir runtimeDir() const;
	bool isTerminalActive() const;
	bool globalTerminal() const;

public Q_SLOTS:
	void quit();
	void reload();

	void setTerminalActive(bool terminalActive);
	void setGlobalTerminal(bool globalTerminal);

Q_SIGNALS:
	void started();
	void stopped(int exitCode);
	void reloaded();
	void paused();
	void resumed();

	void terminalConnected(Terminal *terminal);

	void terminalActiveChanged(bool terminalActive, QPrivateSignal);
	void globalTerminalChanged(bool globalTerminal, QPrivateSignal);

protected:
	virtual bool preStart();

	virtual CommandMode onStart() = 0;
	virtual CommandMode onStop(int &exitCode);
	virtual CommandMode onReload();
	virtual CommandMode onPause();
	virtual CommandMode onResume();

	virtual QVariant onCallback(const QByteArray &kind, const QVariantList &args);

	void addCallback(const QByteArray &kind, const std::function<QVariant(QVariantList)> &fn);
	template <typename TFunction>
	void addCallback(const QByteArray &kind, const TFunction &fn);

private:
	friend class QtService::ServiceBackend;
	QScopedPointer<ServicePrivate> d;
};

template<typename TFunction>
void Service::addCallback(const QByteArray &kind, const TFunction &fn)
{
	addCallback(kind, __helpertypes::pack_function(fn));
}

}

#endif // QTSERVICE_SERVICE_H
