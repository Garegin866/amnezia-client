#include "defines.h"
#include "settings.h"

#include <QDebug>
#include "protocols/protocols_defs.h"

Settings::Settings(QObject* parent) :
    QObject(parent),
    m_settings (ORGANIZATION_NAME, APPLICATION_NAME, this)
{
    // Import old settings
    if (serversCount() == 0) {
        QString user = m_settings.value("Server/userName").toString();
        QString password = m_settings.value("Server/password").toString();
        QString serverName = m_settings.value("Server/serverName").toString();
        int port = m_settings.value("Server/serverPort").toInt();

        if (!user.isEmpty() && !password.isEmpty() && !serverName.isEmpty()){
            QJsonObject server;
            server.insert(config_key::userName, user);
            server.insert(config_key::password, password);
            server.insert(config_key::hostName, serverName);
            server.insert(config_key::port, port);
            server.insert(config_key::description, tr("Server #1"));

            addServer(server);
        }
    }
}

int Settings::serversCount() const
{
    return serversArray().size();
}

QJsonObject Settings::server(int index) const
{
    const QJsonArray &servers = serversArray();
    if (index >= servers.size()) return QJsonObject();

    return servers.at(index).toObject();
}

void Settings::addServer(const QJsonObject &server)
{
    QJsonArray servers = serversArray();
    servers.append(server);
    setServersArray(servers);
}

void Settings::removeServer(int index)
{
    QJsonArray servers = serversArray();
    if (index >= servers.size()) return;

    servers.removeAt(index);
    setServersArray(servers);
}

bool Settings::editServer(int index, const QJsonObject &server)
{
    QJsonArray servers = serversArray();
    if (index >= servers.size()) return false;

    servers.replace(index, server);
    setServersArray(servers);
    return true;
}

void Settings::setDefaultContainer(int serverIndex, DockerContainer container)
{
    QJsonObject s = server(serverIndex);
    s.insert(config_key::defaultContainer, containerToString(container));
    editServer(serverIndex, s);
}

DockerContainer Settings::defaultContainer(int serverIndex) const
{
    return containerFromString(defaultContainerName(serverIndex));
}

QString Settings::defaultContainerName(int serverIndex) const
{
    QString name = server(serverIndex).value(config_key::defaultContainer).toString();
    if (name.isEmpty()) {
        return containerToString(DockerContainer::OpenVpnOverCloak);
    }
    else return name;
}

QMap<DockerContainer, QJsonObject> Settings::containers(int serverIndex) const
{
    const QJsonArray &containers = server(serverIndex).value(config_key::containers).toArray();

    QMap<DockerContainer, QJsonObject> containersMap;
    for (const QJsonValue &val : containers) {
        containersMap.insert(containerFromString(val.toObject().value(config_key::container).toString()), val.toObject());
    }

    return containersMap;
}

void Settings::setContainers(int serverIndex, const QMap<DockerContainer, QJsonObject> &containers)
{
    QJsonObject s = server(serverIndex);
    QJsonArray c;
    for (const QJsonObject &o: containers) {
        c.append(o);
    }
    s.insert(config_key::containers, c);
    editServer(serverIndex, s);
}


QJsonObject Settings::containerConfig(int serverIndex, DockerContainer container)
{
    if (container == DockerContainer::None) return QJsonObject();
    return containers(serverIndex).value(container);
}

void Settings::setContainerConfig(int serverIndex, DockerContainer container, const QJsonObject &config)
{
    if (container == DockerContainer::None) {
        qCritical() << "Settings::setContainerConfig trying to set config for container == DockerContainer::None";
        return;
    }
    auto c = containers(serverIndex);
    c[container] = config;
    c[container][config_key::container] = containerToString(container);
    setContainers(serverIndex, c);
}

void Settings::removeContainerConfig(int serverIndex, DockerContainer container)
{
    if (container == DockerContainer::None) {
        qCritical() << "Settings::removeContainerConfig trying to remove config for container == DockerContainer::None";
        return;
    }

    auto c = containers(serverIndex);
    c.remove(container);
    setContainers(serverIndex, c);
}

QJsonObject Settings::protocolConfig(int serverIndex, DockerContainer container, Protocol proto)
{
    const QJsonObject &c = containerConfig(serverIndex, container);
    return c.value(protoToString(proto)).toObject();
}

void Settings::setProtocolConfig(int serverIndex, DockerContainer container, Protocol proto, const QJsonObject &config)
{
    QJsonObject c = containerConfig(serverIndex, container);
    c.insert(protoToString(proto), config);

    setContainerConfig(serverIndex, container, c);
}

void Settings::clearLastConnectionConfig(int serverIndex, DockerContainer container, Protocol proto)
{
    if (proto == Protocol::Any) {
        for (Protocol p: { Protocol::OpenVpn, Protocol::ShadowSocks, Protocol::Cloak, Protocol::WireGuard}) {
            clearLastConnectionConfig(serverIndex, container, p);
        }
        return;
    }

    QJsonObject c = protocolConfig(serverIndex, container, proto);
    c.remove(config_key::last_config);
    setProtocolConfig(serverIndex, container, proto, c);
    qDebug() << "Settings::clearLastConnectionConfig for" << serverIndex << container << proto;
}

bool Settings::haveAuthData(int serverIndex) const
{
    if (serverIndex < 0) return false;
    ServerCredentials cred = serverCredentials(serverIndex);
    return (!cred.hostName.isEmpty() && !cred.userName.isEmpty() && !cred.password.isEmpty());
}

QString Settings::nextAvailableServerName() const
{
    int i = 0;
    bool nameExist = false;

    do {
        i++;
        nameExist = false;
        for (const QJsonValue &server: serversArray()) {
            if (server.toObject().value(config_key::description).toString() == tr("Server") + " " + QString::number(i)) {
                nameExist = true;
                break;
            }
        }
    } while (nameExist);

    return tr("Server") + " " + QString::number(i);
}

//void Settings::setServerCredentials(const ServerCredentials &credentials)
//{
//    setServerName(credentials.hostName);
//    setServerPort(credentials.port);
//    setUserName(credentials.userName);
//    setPassword(credentials.password);
//}

ServerCredentials Settings::defaultServerCredentials() const
{
    return serverCredentials(defaultServerIndex());
}

ServerCredentials Settings::serverCredentials(int index) const
{
    const QJsonObject &s = server(index);

    ServerCredentials credentials;
    credentials.hostName = s.value(config_key::hostName).toString();
    credentials.userName = s.value(config_key::userName).toString();
    credentials.password = s.value(config_key::password).toString();
    credentials.port = s.value(config_key::port).toInt();

    return credentials;
}
