#include "userconfig.h"
#ifdef LOG_NEW

#include <new>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <sstream>
#include <cstdio>
#include <set>
#include <cmath>
#include <cassert>
#include "newlogger.h"

using namespace std;

std::string toString(int x)
{
    //C++-like: (fehlerhaft: fuehrt zu trailing space)
    //std::stringstream os;
    //os << x << std::ends;
    //std::string s(os.str());
    //return s;
    
    // C-like:
    char s[40];
    sprintf(s, "%d", x);
    return string(s);
}

LogInfo::LogInfo() :
    type(), filepos(), allocCallCount(0), deallocCallCount(0), allocated_mem(0),
    peak_allocated_mem(0)
{
}

LogInfo::LogInfo(const string& type_, const string& filepos_) :
    type(type_), filepos(filepos_), allocCallCount(0), deallocCallCount(0),
    allocated_mem(0), peak_allocated_mem(0)
{
}

void LogInfo::logAllocation(size_t mem)
{
    ++allocCallCount;
    allocated_mem += mem;
    peak_allocated_mem = max(allocated_mem, peak_allocated_mem);
}

void LogInfo::logDeallocation(size_t mem)
{
    ++deallocCallCount;
    assert(mem <= allocated_mem);
    allocated_mem -= mem;
}

size_t LogInfo::getAllocCallCount() const
{
    return allocCallCount;
}

size_t LogInfo::getDeallocCallCount() const
{
    return deallocCallCount;
}

size_t LogInfo::getPeakAllocatedMem() const
{
    return peak_allocated_mem;
}

size_t LogInfo::getAllocatedMem() const
{
    return allocated_mem;
}

bool LogInfo::compare_by_peakmem(const LogInfo* lhs, const LogInfo* rhs)
{
    return lhs->peak_allocated_mem < rhs->peak_allocated_mem;
}

TypeLogInfo::TypeLogInfo() :
    type(), allocCallCount(0), deallocCallCount(0), peak_allocated_mem(0),
    allocated_mem(0)
{
}

bool TypeLogInfo::compare_by_peakmem(const TypeLogInfo& lhs,
    const TypeLogInfo& rhs)
{
    return lhs.peak_allocated_mem < rhs.peak_allocated_mem;
}

PointerInfo::PointerInfo() : allocated_mem(0), logInfo(NULL)
{
}

PointerInfo::PointerInfo(size_t mem, LogInfo* info)
    : allocated_mem(mem), logInfo(info)
{
}

size_t PointerInfo::getAllocatedMem() const
{
    return allocated_mem;
}

LogInfo* PointerInfo::getLogInfo() const
{
    return logInfo;
}

ReportRowFormat::ReportRowFormat() :
    type_length(0), filepos_length(0), callcount_length(0),
    allocated_mem_length(0)
{
}

NewLogger::log_t NewLogger::log;
NewLogger::pointerLog_t NewLogger::pointerLog;

NewLogger::~NewLogger()
{
    for (log_t::const_iterator iter = log.begin(); iter != log.end();
        ++iter)
    {
        delete iter->second;
    }
}

void NewLogger::logAllocation(std::string type, std::string filepos,
    size_t size, const void* pointer)
{
    std::string key = filepos + ':' + type;

    if (log.find(key) == log.end())
    {
        log[key] = new LogInfo(type, filepos);
    }
    
    log[key]->logAllocation(size);

    assert(pointerLog.find(pointer) == pointerLog.end());

    pointerLog[pointer] = PointerInfo(size, log[key]);
}

void NewLogger::logDeallocation(const void* pointer)
{
    // There may be deletes from the C++ library. So just do nothing if we get
    // a pointer for which we have no log information.
    if (pointerLog.find(pointer) == pointerLog.end())
        return;
    
    PointerInfo pinfo = pointerLog.find(pointer)->second;
    pinfo.getLogInfo()->logDeallocation(pinfo.getAllocatedMem()); 
    pointerLog.erase(pointer);
}


void NewLogger::printall()
{
    printall_by_typesize();
}

void NewLogger::printReportLine(const ReportRowFormat& format,
    char line_character)
{
    std::string line =
        "+" + string(format.type_length          + 0, line_character) +
        "+" + string(format.filepos_length       + 0, line_character) +
        "+" + string(format.callcount_length     + 0, line_character) +
        "+" + string(format.allocated_mem_length + 0, line_character) + "+";

    cerr << line << endl;
}

void NewLogger::printReportDoubleLine(const ReportRowFormat& format)
{
    printReportLine(format, '=');
}

void NewLogger::printall_by_typesize()
{
    // column names of report table
    std::string table_head1_type      = "";
    std::string table_head1_filepos   = "";
    std::string table_head1_callcount = "alloc calls";
    std::string table_head1_mem       = "peak allocated mem";
    std::string table_head2_type      = "type";
    std::string table_head2_filepos   = "filepos";
    std::string table_head2_callcount = "(w/o dealloc)";
    std::string table_head2_mem       = "(not deallocated)";

    // string to mark memory allocations for unknown type
    std::string table_type_no_type = "<no type>";

    // Format of report table. Saves column widths.
    ReportRowFormat format;
    format.type_length =
        max(max(table_head1_type.size(), table_head2_type.size()),
            table_type_no_type.size());

    format.filepos_length =
        max(table_head1_filepos.size(), table_head2_filepos.size());
    
    format.callcount_length =
        max(table_head1_callcount.size(), table_head2_callcount.size());

    format.allocated_mem_length =
        max(table_head1_mem.size(), table_head2_mem.size());
    
    // Needed to determine column widths for callcount and memory column.
    size_t max_callcount     = 0;
    size_t max_allocated_mem = 0;
    
    // We use a multiset to sort memory allocation info by memory allocated.
    typedef bool (*log_by_size_compare_t)(const LogInfo*, const LogInfo*);
    typedef multiset<LogInfo*, log_by_size_compare_t> log_by_size_t;
    log_by_size_t log_by_size(&LogInfo::compare_by_peakmem);

    // Saves total peak allocated memory.
    size_t total_peak_allocated_mem = 0;

    // Saves total (still) allocated memory, that is, not deallocated memory.
    size_t total_allocated_mem = 0;

    // Saves total number of allocation calls.
    size_t total_alloccallcount = 0;
    
    // Saves total number of deallocation calls.
    size_t total_dealloccallcount = 0;
    
    // determine allocated memory per type and sort by size (descending)
    typedef std::map<std::string, TypeLogInfo> typelog_t;
    typelog_t typelog;
    for (log_t::const_iterator iter = log.begin(); iter != log.end();
        ++iter)
    {
        log_by_size.insert(iter->second);

        total_peak_allocated_mem += iter->second->getPeakAllocatedMem();
        total_allocated_mem      += iter->second->getAllocatedMem();
        total_alloccallcount     += iter->second->getAllocCallCount();
        total_dealloccallcount   += iter->second->getDeallocCallCount();

        format.filepos_length =
            max(iter->second->filepos.size(), format.filepos_length);
        
        typelog[iter->second->type].type = iter->second->type;

        typelog[iter->second->type].peak_allocated_mem +=
            iter->second->getPeakAllocatedMem();
       
        typelog[iter->second->type].allocated_mem +=
            iter->second->getAllocatedMem();
       
        typelog[iter->second->type].allocCallCount +=
            iter->second->getAllocCallCount();
        
        typelog[iter->second->type].deallocCallCount +=
            iter->second->getDeallocCallCount();
    }

    // Another multimap to sort per type memory allocation info by size.
    typedef bool (*typelog_by_size_compare_t)(const TypeLogInfo&,
        const TypeLogInfo&);
    
    typedef multiset<TypeLogInfo, typelog_by_size_compare_t> typelog_by_size_t;
    typelog_by_size_t typelog_by_size(&TypeLogInfo::compare_by_peakmem);
    
    for (typelog_t::const_iterator iter = typelog.begin();
        iter != typelog.end(); ++iter)
    {
        typelog_by_size.insert(iter->second);
        
        format.type_length = max(iter->second.type.size(), format.type_length);
        max_callcount      = max(max_callcount, iter->second.allocCallCount);
        max_allocated_mem  = max(max_allocated_mem,
            iter->second.peak_allocated_mem);
    }
    
    // determine space needed for call count column
    format.callcount_length =
        max((size_t)floor(log10((double) max_callcount)) + 1, format.callcount_length);

    // determine space needed for allocated memory column
    format.allocated_mem_length = max((size_t)floor(log10((double) max_allocated_mem))+1,
        format.allocated_mem_length);

    // Right justify table cells.
    cerr << setiosflags(ios_base::right);

    // Print table head.
    printReportDoubleLine(format);
    printReportRow(table_head1_type, table_head1_filepos, table_head1_callcount,
        table_head1_mem, format);
    printReportRow(table_head2_type, table_head2_filepos, table_head2_callcount,
        table_head2_mem, format);
    printReportDoubleLine(format);
    

    // For each type ...
    for (typelog_by_size_t::const_reverse_iterator typeiter =
         typelog_by_size.rbegin(); typeiter != typelog_by_size.rend();
         ++typeiter)
    {
        // Print summary for current type.
        string print_type = typeiter->type.size() != 0 ?
                                 typeiter->type : table_type_no_type;
        
        printReportRow(print_type, "total", typeiter->allocCallCount,
            typeiter->deallocCallCount, typeiter->peak_allocated_mem,
            typeiter->allocated_mem, format);
        
        std::string current_type = typeiter->type;

        // Print "file:line"-entries for current type
        for (log_by_size_t::const_reverse_iterator iter = log_by_size.rbegin();
             iter != log_by_size.rend(); ++iter)
        {
            if ((*iter)->type != current_type)
                continue;

            printReportRow("", (*iter)->filepos, (*iter)->getAllocCallCount(),
                (*iter)->getDeallocCallCount(), (*iter)->getPeakAllocatedMem(),
                (*iter)->getAllocatedMem(), format);
        }

        printReportLine(format);
    }

    cerr << endl;
    cerr << "total memory usage: " << total_peak_allocated_mem << " bytes; "
         << total_alloccallcount << " total allocation calls" << endl;
    cerr << "(total memory usage in MB: "
         << (total_peak_allocated_mem/1024/1024) << ")" << endl;
    cerr << "total not deallocated memory: " << total_allocated_mem
         << " bytes; " << total_alloccallcount - total_dealloccallcount
         << " allocs w/o matching deallocs" << endl;
}

void* mynew(size_t size, const std::string &file, int line)
{
    //std::cout << file << ": " << line << std::endl;
    std::string filepos(file);
    filepos += ':';
    filepos += toString(line);
    void* ptr = malloc(size);
    NewLogger::logAllocation("", filepos, size, ptr); 
    return ptr;
}

void * operator new (size_t size, const std::string &file, int line)
{
    return mynew(size, file, line);
}

void * operator new[] (size_t size, const std::string &file, int line)
{
    return mynew(size, file, line);
}

void mydelete(void* mem)
{
    NewLogger::logDeallocation(mem);
    free(mem);
}

void operator delete (void* mem)
{
    mydelete(mem);
}

void operator delete[] (void* mem)
{
    mydelete(mem);
}

#endif // LOG_NEW
