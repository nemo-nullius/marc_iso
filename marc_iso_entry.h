#include "decl.h"
#define NDEBUG ;
//#define NTRYCATCH

#ifndef STRU_RAWISOPOS
#define STRU_RAWISOPOS

struct RawIsoPos
{
    pair<size_t, size_t> ldr{0, 24}; // [begin_pos, end_pos)
    pair<size_t, size_t> header{24, 0};
    pair<size_t, size_t> body{0, 0};
};

// ATTENTION:
// One Chinese character is saved in 2 bytes
// Code: GB2312
struct MarcIsoEntry
{

public:
    MarcIsoEntry(const string s) : raw(s)
    { // assign to rawpos
        auto body_beg_pos = s.find("\x1e");
        rawpos.header.second = rawpos.body.first = body_beg_pos;
        rawpos.body.second = s.length();
        recordid_from_raw = s.substr(body_beg_pos + 1, 9);
    };
    string entry_one(const string);
    mmm get_fields()
    {
        // if fields has not been made, make it. // TODO: seems useless check
        if (this->fields.size() == 0)
        {
#ifndef NTRYCATCH
            try
            {
#endif
                makefields();
#ifndef NTRYCATCH
            }
            catch (const std::exception &e)
            {
                err_data_flg = true;
                cerr << e.what() << endl;
                cerr << recordid_from_raw << "\t[ABORT] Error happened. Process of this record is aborted." << endl;
            }
            catch (...)
            {
                err_data_flg = true;
                cerr << recordid_from_raw << "\t[ABORT] Unknown error. Process of this record is aborted." << endl;
            }
#endif
        }
        return fields;
    }; // make it into a map
    void output_raw_fields();
    string get_raw_data() { return raw; };
    bool is_bad_data() { return bad_data_flg; }; // assign bad_data to string
    bool is_err_data() { return err_data_flg; }; // assign bad_data to string

private:
    string raw;                // original raw marc iso data in the format of a string
    RawIsoPos rawpos;          // assigned when *this initiates
    mmm fields;                // from parser_rawbody()
    mmm fields_virt;           // have a max range of keys for output in csv
    bool bad_data_flg = false; // from makefields()
    bool err_data_flg = false;
    string recordid_from_raw; // Record ID got from raw directly
    vector<size_t> findall(const string s, const string to_found_s, const size_t start_pos = 0)
    {
        vector<size_t> result;
        size_t found_pos = start_pos;
        while ((found_pos = s.find(to_found_s, found_pos)) != std::string::npos)
        {
            result.push_back(found_pos);
            ++found_pos;
        }
        return result;
    }
    void makefields() // append fields, return a set of bad data (append)
    {
        map<string, string> subfields; // subfields
        // get & insert
        // insert ldr
        string rawldr = string(raw, rawpos.ldr.first, rawpos.ldr.second - rawpos.ldr.first);
        subfields["NUL"] = rawldr;
        fields.insert(make_pair("ldr", subfields));
        subfields.clear();
        // check data length
        size_t raw_len_real = raw.size();
        size_t raw_len_virt;
        // no bad_data_flag or err_data_flag defined here
        if (stoi_safe(string(rawldr.begin(), rawldr.begin() + 5), &raw_len_virt) < 0)
        {
            cerr << recordid_from_raw << "\t[WAR] Literal data length not obtainable." << endl;
        }
        else if (raw_len_virt != raw_len_real)
        {
            cerr << recordid_from_raw << "\t[WAR] Literal data length not consistent with actual data length (" << raw_len_virt
                 << " " << raw_len_real << ")" << endl;
        }

        // insert body subfields
        string rawheader = string(raw, rawpos.header.first, rawpos.header.second - rawpos.header.first);
        string rawbody = string(raw, rawpos.body.first, rawpos.body.second - rawpos.body.first);
#ifndef NDEBUG
        cerr << recordid_from_raw << "\t" << rawldr << endl;
        cerr << recordid_from_raw << "\t" << rawheader << endl;
        cerr << recordid_from_raw << "\t" << rawbody << endl;
#endif
        // check data integrity
        if (!rawheader.length() % 12)
            throw runtime_error(recordid_from_raw + "\t[ERR] Header content corrupted.");
        if (rawbody[0] != '\x1e')
            throw runtime_error(recordid_from_raw + "\t[ERR] Body content corrupted.");
        // get all fields (fld_raw_real) by '\x1e'
        // IMPORTANT: to make sure there is a '\x1e' at end of the string
        // to make viable the split of '\x1e' to get fields
        // Normal record would end with "\x1e\x1d"
        rawbody = (string(rawbody.end() - 2, rawbody.end()) == string("\x1e\x1d")) ? rawbody : rawbody + "\x1e\x1d";
        vector<string> flds_raw_real;
        auto flds_raw_real_pos = this->findall(rawbody, "\x1e", 0);
        for (auto beg = flds_raw_real_pos.begin(); beg != flds_raw_real_pos.end() - 1; ++beg) // left over final "\x1e\x1d"
            flds_raw_real.push_back(string(rawbody.begin() + *beg, rawbody.begin() + *(beg + 1)));

        if ((rawheader.length() / 12) != flds_raw_real.size())
        {
            cerr << rawheader.length() << " " << flds_raw_real.size() << endl;
            throw runtime_error(recordid_from_raw + "\t[ERR] Header info not consistent with fields number");
        }
        // loop over the header info of each field
        // [field_number(3), field_length(4), field_beginning_position(5)]
        for (size_t i = 0; i != rawheader.length(); i += 12)
        {
            string field_header_info = string(rawheader, i, 12);
            string fld_num = string(field_header_info, 0, 3);
            string fld_len = string(field_header_info, 3, 4);
            string fld_bpos = string(field_header_info, 7, 5);
            string fld_raw_real = flds_raw_real[i / 12];
            auto fld_bpos_ul = stoul(fld_bpos);
            auto fld_len_ul = stoul(fld_len);
            // check fld_raw_virt to exmaine data condition
            if (rawbody.size() >= fld_bpos_ul && rawbody.size() >= fld_bpos_ul + fld_len_ul)
            {
                string fld_raw_virt = string(rawbody, stoul(fld_bpos), stoul(fld_len));
                if (fld_raw_virt != fld_raw_real)
                {
                    cerr << recordid_from_raw << "\t[WAR] Corrupt data. ";
                    cerr << "[WAR] VIRT VS REAL: " << fld_raw_virt << " " << fld_raw_real << endl;
                    bad_data_flg = true;
                }
            }
            else
            {
                //cerr << "data size: " << raw.size() << endl;
                //cerr << "bodydata size: " << rawbody.size() << endl;
                cerr << recordid_from_raw << "\t[WAR] Corrupt data. Body data lost." << endl;
                bad_data_flg = true;
            }
            string fld_raw = fld_raw_real;

#ifndef NDEBUG
            cerr << recordid_from_raw << "\t" << field_header_info << " " << fld_num << " " << fld_len << " " << fld_bpos << " " << fld_raw << endl;
#endif
            // check fld_raw
            if (fld_raw[0] != '\x1e' ||
                (fld_raw[3] != '\x1f' && fld_num != "001" && fld_num != "005"))
            {
                cerr << fld_raw << endl;
                throw runtime_error(recordid_from_raw + "\t[ERR] Subfield beginning char corrupted.");
            }
            // insert subfields
            if (fld_num == "001" || fld_num == "005")
            {
                // no subfield
                subfields["NUL"] = string(fld_raw.begin() + 1, fld_raw.end()); // jump over "\x1e"
            }
            else
            {
                // one or more than one subfields
                //[subfield_flag(3):\x1eAB, subfield_text(...):\x1fa...\x1fb...]
                string fld_flg = string(fld_raw.begin() + 1, fld_raw.begin() + 3);
                subfields["_FLG"] = fld_flg;
                auto subf_pos_all = this->findall(fld_raw, "\x1f", 0);
                for (auto beg = subf_pos_all.begin(); beg != subf_pos_all.end() - 1; ++beg)
                {
                    string k = string(fld_raw.begin() + *beg + 1, fld_raw.begin() + *beg + 2);
                    string v = string(fld_raw.begin() + *beg + 2, fld_raw.begin() + *(beg + 1));
                    subfields[k] = v;
                }
                subfields[string(fld_raw.begin() + *(subf_pos_all.end() - 1) + 1, fld_raw.begin() + *(subf_pos_all.end() - 1) + 2)] = string(fld_raw.begin() + *(subf_pos_all.end() - 1) + 2, fld_raw.end());
            }
            fields.insert(make_pair(fld_num, subfields));
            subfields.clear();
        }
    }; //makefields()
};

inline void MarcIsoEntry::output_raw_fields()
{
    for (auto &fld : this->fields)
    {
        auto fk = fld.first;
        cout << fk << ":" << endl;
        for (auto &subf : fld.second)
        {
            auto sfk = subf.first;
            auto sfv = subf.second;
            cout << "\t" << sfk << "\t" << sfv << endl;
        }
    }
}
#endif
