//--------------------------------------------------------------------------------
//
// Filename    : CLRegisterPlayer.h
// Written By  : Reiot
//
//--------------------------------------------------------------------------------

#ifndef __CL_REGISTER_PLAYER_H__
#define __CL_REGISTER_PLAYER_H__

// include files
#include "Packet.h"
#include "PacketFactory.h"
#include "WireString.h"

//--------------------------------------------------------------------------------
//
// class CLRegisterPlayer;
//
// The first packet the client sends to the login server.
// The id and the password are encrypted. Not yet, though..
//
//--------------------------------------------------------------------------------

class CLRegisterPlayer : public Packet {
public:
    CLRegisterPlayer(){};
    virtual ~CLRegisterPlayer(){};
    // Read data from the input stream (buffer) and initialise the packet.
    void read(SocketInputStream& iStream);

    // Send the packet's binary image to the output stream (buffer).
    void write(SocketOutputStream& oStream) const;


    // get packet id
    PacketID_t getPacketID() const {
        return PACKET_CL_REGISTER_PLAYER;
    }

    // get packet's body size
    PacketSize_t getPacketSize() const {
        // When optimizing, use the precomputed constant.
        return de::wire::stringWireSize(m_ID)          // Id
               + de::wire::stringWireSize(m_Password)  // Password
               + de::wire::stringWireSize(m_Name)      // Name
               + szBYTE                                // Sex
               + de::wire::stringWireSize(m_SSN)       // Resident registration number
               + de::wire::stringWireSize(m_Telephone) // Telephone number
               + de::wire::stringWireSize(m_Cellular)  // Mobile phone number
               + de::wire::stringWireSize(m_ZipCode)   // Zip code
               + de::wire::stringWireSize(m_Address)   // Address
               + szBYTE                                // Country code
               + de::wire::stringWireSize(m_Email)     // Email
               + de::wire::stringWireSize(m_Homepage)  // Homepage
               + de::wire::stringWireSize(m_Profile)   // Profile text
               + szBYTE;                               // Whether it is public
    }

    // get packet name
    string getPacketName() const {
        return "CLRegisterPlayer";
    }

    // get packet's debug string
    string toString() const;

public:
    //----------------------------------------------------------------------
    // *CAUTION*
    // Each setXXX() checks the maximum length and truncates, but does not check
    // the minimum. The minimum length is checked in read()/write().
    //----------------------------------------------------------------------

    // get/set player's id
    string getID() const {
        return m_ID;
    }
    void setID(string id) {
        m_ID = (id.size() > maxIDLength) ? id.substr(0, maxIDLength) : id;
    }

    // get/set player's password
    string getPassword() const {
        return m_Password;
    }
    void setPassword(string password) {
        m_Password = (password.size() > maxPasswordLength) ? password.substr(0, maxPasswordLength) : password;
    }

    // get/set player's name
    string getName() const {
        return m_Name;
    }
    void setName(string name) {
        m_Name = (name.size() > maxNameLength) ? name.substr(0, maxNameLength) : name;
    }

    // get/set player's sex
    Sex getSex() const {
        return m_Sex;
    }
    void setSex(Sex sex) {
        m_Sex = sex;
    }

    // get/set player's ssn
    string getSSN() const {
        return m_SSN;
    }
    void setSSN(string ssn) {
        m_SSN = (ssn.size() > maxSSNLength) ? ssn.substr(0, maxSSNLength) : ssn;
    }

    // get/set player's telephone
    string getTelephone() const {
        return m_Telephone;
    }
    void setTelephone(string telephone) {
        m_Telephone = (telephone.size() > maxTelephoneLength) ? telephone.substr(0, maxTelephoneLength) : telephone;
    }

    // get/set player's cellular
    string getCellular() const {
        return m_Cellular;
    }
    void setCellular(string cellular) {
        m_Cellular = (cellular.size() > maxCellularLength) ? cellular.substr(0, maxCellularLength) : cellular;
    }

    // get/set player's zipcode
    string getZipCode() const {
        return m_ZipCode;
    }
    void setZipCode(string zipcode) {
        m_ZipCode = (zipcode.size() > maxZipCodeLength) ? zipcode.substr(0, maxZipCodeLength) : zipcode;
    }

    // get/set player's address
    string getAddress() const {
        return m_Address;
    }
    void setAddress(string address) {
        m_Address = (address.size() > maxAddressLength) ? address.substr(0, maxAddressLength) : address;
    }

    // get/set player's nation
    Nation getNation() const {
        return m_Nation;
    }
    void setNation(Nation nation) {
        m_Nation = nation;
    }

    // get/set player's email
    string getEmail() const {
        return m_Email;
    }
    void setEmail(string email) {
        m_Email = (email.size() > maxEmailLength) ? email.substr(0, maxEmailLength) : email;
    }

    // get/set player's homepage
    string getHomepage() const {
        return m_Homepage;
    }
    void setHomepage(string homepage) {
        m_Homepage = (homepage.size() > maxHomepageLength) ? homepage.substr(0, maxHomepageLength) : homepage;
    }

    // get/set player's profile
    string getProfile() const {
        return m_Profile;
    }
    void setProfile(string profile) {
        m_Profile = (profile.size() > maxProfileLength) ? profile.substr(0, maxProfileLength) : profile;
    }

    // get/set player info's publicability(?)
    bool getPublic() const {
        return m_bPublic;
    }
    void setPublic(bool bPublic) {
        m_bPublic = bPublic;
    }

private:
    //--------------------------------------------------
    // Player's basic information
    //--------------------------------------------------
    string m_ID;        // Id
    string m_Password;  // Password
                        //--------------------------------------------------
                        // Player's personal information
                        //--------------------------------------------------
    string m_Name;      // Name
    Sex m_Sex;          // Sex
    string m_SSN;       // Resident registration number
                        //--------------------------------------------------
                        // Player's contact details and address
                        //--------------------------------------------------
    string m_Telephone; // Telephone number
    string m_Cellular;  // Mobile phone
    string m_ZipCode;   // Zip code
    string m_Address;   // Address
    Nation m_Nation;    // Country code
                        //--------------------------------------------------
                        // Player's electronic details
                        //--------------------------------------------------
    string m_Email;     // Email
    string m_Homepage;  // Homepage
                        //--------------------------------------------------
                        // Other
                        //--------------------------------------------------
    string m_Profile;   // What the player wants to say
    bool m_bPublic;     // Whether it is public
};


//--------------------------------------------------------------------------------
//
// class CLRegisterPlayerFactory;
//
// Factory for CLRegisterPlayer
//
//--------------------------------------------------------------------------------

class CLRegisterPlayerFactory : public PacketFactory {
public:
    static constexpr PacketID_t kPacketID = Packet::PACKET_CL_REGISTER_PLAYER;
    static constexpr std::string_view kName = "CLRegisterPlayer";
    // When optimizing, use the precomputed constant.
    static constexpr PacketSize_t kMaxSize{szBYTE + maxIDLength          // Id
                                           + szBYTE + maxPasswordLength  // Password
                                           + szBYTE + maxNameLength      // Name
                                           + szBYTE                      // Sex
                                           + szBYTE + maxSSNLength       // Resident registration number
                                           + szBYTE + maxTelephoneLength // Telephone number
                                           + szBYTE + maxCellularLength  // Mobile phone number
                                           + szBYTE + maxZipCodeLength   // Zip code
                                           + szBYTE + maxAddressLength   // Address
                                           + szBYTE                      // Country code
                                           + szBYTE + maxEmailLength     // Email
                                           + szBYTE + maxHomepageLength  // Homepage
                                           + szBYTE + maxProfileLength   // Profile
                                           + szBYTE};                    // Whether it is public

    // create packet
    Packet* createPacket() override {
        return new CLRegisterPlayer();
    }

    // get packet name
    string getPacketName() const override {
        return string(kName);
    }

    // get packet id
    PacketID_t getPacketID() const override {
        return kPacketID;
    }

    // get packet's max body size
    PacketSize_t getPacketMaxSize() const override {
        return kMaxSize;
    }
};


//--------------------------------------------------------------------------------
//
// class CLRegisterPlayerHandler;
//
//--------------------------------------------------------------------------------

class CLRegisterPlayerHandler {
public:
    // execute packet's handler
    static void execute(CLRegisterPlayer* pPacket, Player* pPlayer);
};

#endif
