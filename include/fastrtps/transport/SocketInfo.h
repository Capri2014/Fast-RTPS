#ifndef RTPS_SOCKET_INFO_
#define RTPS_SOCKET_INFO_

#include <asio.hpp>
#include <memory>
#include <map>
#include <fastrtps/rtps/messages/MessageReceiver.h>
#include <fastrtps/log/Log.h>

namespace eprosima{
namespace fastrtps{
namespace rtps{

#if defined(ASIO_HAS_MOVE)
    // Typedefs
	typedef asio::ip::udp::socket eProsimaUDPSocket;
	typedef asio::ip::tcp::socket eProsimaTCPSocket;
    // UDP
	inline eProsimaUDPSocket* getSocketPtr(eProsimaUDPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaUDPSocket moveSocket(eProsimaUDPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::udp::socket(io_service));
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket* socket)
    {
        return *socket;
    }
    // TCP
	inline eProsimaTCPSocket* getSocketPtr(eProsimaTCPSocket &socket)
    {
        return &socket;
    }
    inline eProsimaTCPSocket moveSocket(eProsimaTCPSocket &socket)
    {
        return std::move(socket);
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::move(asio::ip::tcp::socket(io_service));
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket* socket)
    {
        return *socket;
    }
#else
    // Typedefs
	typedef std::shared_ptr<asio::ip::udp::socket> eProsimaUDPSocket;
	typedef std::shared_ptr<asio::ip::tcp::socket> eProsimaTCPSocket;
    // UDP
    inline eProsimaUDPSocket getSocketPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket& moveSocket(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    inline eProsimaUDPSocket createUDPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::udp::socket>(io_service);
    }
    inline eProsimaUDPSocket& getRefFromPtr(eProsimaUDPSocket &socket)
    {
        return socket;
    }
    // TCP
    inline eProsimaTCPSocket getSocketPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket& moveSocket(eProsimaTCPSocket &socket)
    {
        return socket;
    }
    inline eProsimaTCPSocket createTCPSocket(asio::io_service& io_service)
    {
        return std::make_shared<asio::ip::tcp::socket>(io_service);
    }
    inline eProsimaTCPSocket& getRefFromPtr(eProsimaTCPSocket &socket)
    {
        return socket;
    }
#endif

class SocketInfo
{
public:
    SocketInfo();
    SocketInfo(uint32_t rec_buffer_size);
    virtual ~SocketInfo();

    inline void SetThread(std::thread* pThread)
    {
        mThread = pThread;
    }

    inline void SetAutoRelease(bool bRelease)
    {
        mAutoRelease = bRelease;
    }

    std::thread* ReleaseThread();

    inline bool IsAlive() const
    {
        return mAlive;
    }

    inline void Disable()
    {
        mAlive = false;
    }

    inline CDRMessage_t& GetMessageBuffer()
    {
        return m_rec_msg;
    }

protected:
    //!Received message
    CDRMessage_t m_rec_msg;
#if HAVE_SECURITY
    CDRMessage_t m_crypto_msg;
#endif

    bool mAlive;
    std::thread* mThread;
    bool mAutoRelease;
};

class UDPSocketInfo : public SocketInfo
{
public:
    UDPSocketInfo(eProsimaUDPSocket& socket);
    UDPSocketInfo(eProsimaUDPSocket& socket, uint32_t maxMsgSize);
    UDPSocketInfo(UDPSocketInfo&& socketInfo);
    virtual ~UDPSocketInfo();

    UDPSocketInfo& operator=(UDPSocketInfo&& socketInfo)
    {
        socket_ = moveSocket(socketInfo.socket_);
        return *this;
    }

    void only_multicast_purpose(const bool value)
    {
        only_multicast_purpose_ = value;
    };

    bool& only_multicast_purpose()
    {
        return only_multicast_purpose_;
    }

    bool only_multicast_purpose() const
    {
        return only_multicast_purpose_;
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaUDPSocket* getSocket()
#else
    inline eProsimaUDPSocket getSocket()
#endif
    {
        return getSocketPtr(socket_);
    }

    inline void SetMessageReceiver(std::shared_ptr<MessageReceiver> receiver)
    {
        mMsgReceiver = receiver;
    }

    inline std::shared_ptr<MessageReceiver> GetMessageReceiver()
    {
        return mMsgReceiver;
    }

private:

    std::shared_ptr<MessageReceiver> mMsgReceiver; //Associated Readers/Writers inside of MessageReceiver
    eProsimaUDPSocket socket_;
    bool only_multicast_purpose_;
    UDPSocketInfo(const UDPSocketInfo&) = delete;
    UDPSocketInfo& operator=(const UDPSocketInfo&) = delete;
};

class TCPSocketInfo : public SocketInfo
{
enum eConnectionStatus
{
    eDisconnected = 0,
    eConnected,                 // Output -> Send bind message.
    eWaitingForBind,            // Input -> Waiting for the bind message.
    eWaitingForBindResponse,    // Output -> Waiting for the bind response message.
    eEstablished,
    eUnbinding
};

public:
    TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
        bool autoRelease);

    TCPSocketInfo(eProsimaTCPSocket& socket, Locator_t& locator, bool outputLocator, bool inputSocket,
        bool autoRelease, uint32_t maxMsgSize);

    TCPSocketInfo(TCPSocketInfo&& socketInfo);

    virtual ~TCPSocketInfo();

    TCPSocketInfo& operator=(TCPSocketInfo&& socketInfo)
    {
        mSocket = moveSocket(socketInfo.mSocket);
        return *this;
    }

    bool operator==(const TCPSocketInfo& socketInfo) const
    {
        return &mSocket == &(socketInfo.mSocket);
    }

#if defined(ASIO_HAS_MOVE)
    inline eProsimaTCPSocket* getSocket()
#else
    inline eProsimaTCPSocket getSocket()
#endif
    {
        return getSocketPtr(mSocket);
    }

    std::recursive_mutex& GetReadMutex() const
    {
        return *mReadMutex;
    }

    std::recursive_mutex& GetWriteMutex() const
    {
        return *mWriteMutex;
    }

    inline void SetRTCPThread(std::thread* pThread)
    {
        mRTCPThread = pThread;
    }

    std::thread* ReleaseRTCPThread();

    inline void SetPhysicalPort(uint16_t port)
    {
        m_physicalPort = port;
    }

    inline uint16_t GetPhysicalPort() const
    {
        return m_physicalPort;
    }

    inline bool GetIsInputSocket() const
    {
        return m_inputSocket;
    }

    inline void SetIsInputSocket(bool bInput)
    {
        m_inputSocket = bInput;
    }

    bool IsConnectionEstablished()
    {
        return mConnectionStatus == eConnectionStatus::eEstablished;
    }

    bool AddMessageReceiver(uint16_t logicalPort, std::shared_ptr<MessageReceiver> receiver);

    std::shared_ptr<MessageReceiver> GetMessageReceiver(uint16_t logicalPort);

    inline const Locator_t& GetLocator() const
    {
        return mLocator;
    }
protected:
    inline void ChangeStatus(eConnectionStatus s)
    {
        mConnectionStatus = s;
    }

    friend class TCPv4Transport;
    friend class RTCPMessageManager;

private:
    Locator_t mLocator;
    uint16_t m_physicalPort;
    bool m_inputSocket;
    bool mWaitingForKeepAlive;
    uint16_t mPendingLogicalPort;
    std::thread* mRTCPThread;
    std::vector<uint16_t> mPendingLogicalOutputPorts;
    std::vector<uint16_t> mLogicalOutputPorts;
    std::vector<uint16_t> mLogicalInputPorts;
    std::shared_ptr<std::recursive_mutex> mReadMutex;
    std::shared_ptr<std::recursive_mutex> mWriteMutex;
    std::map<uint16_t, std::shared_ptr<MessageReceiver>> mReceiversMap;  // The key is the logical port.
    eProsimaTCPSocket mSocket;
    eConnectionStatus mConnectionStatus;
    TCPSocketInfo(const TCPSocketInfo&) = delete;
    TCPSocketInfo& operator=(const TCPSocketInfo&) = delete;
};


} // namespace rtps
} // namespace fastrtps
} // namespace eprosima

#endif // RTPS_TCP_PORT_MANAGER_