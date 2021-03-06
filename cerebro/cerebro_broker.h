#pragma once
#include <list>
#include "util/common.h"
#include "util/logdef.h"
#include "cerebro_struct.h"

class CerebroAccountWrap
{
  public:
  CerebroAccountWrap(int64_t id, const std::string& name);

  int64_t id() const { return _aid;}
  const  std::string& name() const { return _name;}


    // 添加资金, 卖出，入金，分红 使用此接口
    /**
     * @brief 
     * 
     * @param cash 交易资金(不包含费用)
     * @param fee 交易费用
     */
    void add_cash(double cash, double fee = 0.0) {
        _account.cash = _account.cash + cash - fee;
        _account.transaction_cost += fee;
    }

    // 初始化资金
    void set_cash(double cash) { _cash = cash; }
    double get_cash() const { return _cash; }

    void set_slippage(double slippage) { _slippage = slippage; }
    double get_slippage() const { return _slippage; }

    void set_commission_rate(double commission_rate) { _commission_rate = commission_rate; }
    double get_commission_rate() const { return _commission_rate; }

    double calc_commission(const CerebroOrder &order) const
    {
        return (order.filled_price * order.filled_quantity) * _commission_rate;
    }

    double calc_frozen_cash(const CerebroOrder& order, double last_price = 0.0)
    {
        // 计算需冻结的资金
        return (order.price + _slippage) * order.quantity;
    }

    // 解除冻结, 成交后解除冻结（冻结资金-成交金额）
    double unfreeze_cash(double cash)
    {
        if(cash > _account.frozen_cash)
        {
            _account.frozen_cash = 0.0;
            return 0.0;
        }
        _account.frozen_cash -= cash;
        return _account.frozen_cash;
    }

    // 冻结资金, 收到订单后先冻结资金
    void add_frozen_cash(double cash)
    {
        // 冻结部分资金
        _account.frozen_cash += cash;
        // 可用资金减少
        _account.cash += cash;
    }


    // 减少资金, 买入
    /**
     * @brief 
     * 
     * @param cash 交易资金(不包含费用)
     * @param fee  交易费用
     */
    int cost_cash(double cash, double fee)
    {
        if((_account.frozen_cash + _account.cash) < (cash + fee))
        {
            // 资金不足
            return -1;
        }

        // 先消费冻结的资金
        _account.frozen_cash = _account.frozen_cash - cash -fee;

        // 冻结资金不够，再消费可用资金
        if(_account.frozen_cash < 0)
        {
            _account.cash += _account.frozen_cash;
            _account.frozen_cash = 0.0;
        }

        // 增加交易费用
        _account.transaction_cost += fee;
    }


  private:
  std::string _name; // 账户名 
  int64_t _aid {0};// 账户ID 
      double _cash{0};                 // 资金
    double _slippage{0};             // 滑点
    double _commission_rate{0.0003}; // 手续费
    CerebroAccount _account;

};

// 撮合器， 一个交易所一个撮合器， 内部再按品种分不同处理模块
class CerebroMatcher
{
  public:
    CerebroMatcher(CerebroBroker *broker);
    virtual ~CerebroMatcher();
    virtual int match(CerebroOrder &order, const CerebroTickRecord &tick) = 0;
    // int do_order_update(const CerebroOrder& order);
  protected:
    int process_limit_order(CerebroOrder &order, const CerebroTickRecord &tick);
    int process_market_order(CerebroOrder &order, const CerebroTickRecord &tick);
    int process_order_amend(CerebroOrder &order, const CerebroTickRecord &tick);
    int process_order_cancel(CerebroOrder &order, const CerebroTickRecord &tick);

  private:
    CerebroBroker *_broker;
};

// A股撮合器
class CerebroSSEMatcher : public CerebroMatcher
{
  public:
    CerebroSSEMatcher(CerebroBroker *broker);
    virtual ~CerebroSSEMatcher();
    virtual int match(CerebroOrder &order, const CerebroTickRecord &tick) override;

  private:
};

// 期货撮合器
class CerebroSZSEMatcher : public CerebroMatcher
{
  public:
    CerebroSZSEMatcher(CerebroBroker *broker);
    virtual ~CerebroSZSEMatcher();
    virtual int match(CerebroOrder &order, const CerebroTickRecord &tick) override;

  private:
};

class CerebroPositionTracker
{
  public:
    ~CerebroPositionTracker();
    CerebroPositionTracker(const Symbol &symbol);
    // void update(double quantity,double price, double commission);
    void buy(double quantity, double price, double commission = 0.0);
    void sell(double quantity, double price, double commission = 0.0);

    void buy_to_cover(double quantity, double price, double commission = 0.0);
    void sell_short(double quantity, double price, double commission = 0.0);

  private:
    Symbol _symbol;
    CerebroPosition _lposition; //多
    CerebroPosition _sposition; //空
    //
};

// 一个用户一个Broker, 用户下可以有多个资金账户
class CerebroBroker
{
  public:
    CerebroBroker();
    virtual ~CerebroBroker();

    int init(const CerebroConfig& conf);

    // 添加资金, 卖出，入金，分红 使用此接口
    /**
     * @brief 
     * 
     * @param cash 交易资金(不包含费用)
     * @param fee 交易费用
     */
    inline void add_cash(double cash, double fee = 0.0) {
        _account->add_cash(cash, fee);
    }

    // 初始化资金
    inline void set_cash(double cash) { _account->set_cash(cash); }

    inline double get_cash() const { return _account->get_cash(); }
    inline double get_slippage() const { return _account->get_slippage(); }
    inline double get_commission_rate() const { return _account->get_commission_rate(); }



    // 解除冻结, 成交后解除冻结（冻结资金-成交金额）
    inline double unfreeze_cash(double cash)
    {
        return _account->unfreeze_cash(cash);
    }

    // 冻结资金, 收到订单后先冻结资金
    inline void add_frozen_cash(double cash)
    {
      _account->add_frozen_cash(cash);
    }


    // 减少资金, 买入
    /**
     * @brief 
     * 
     * @param cash 交易资金(不包含费用)
     * @param fee  交易费用
     */
    inline int cost_cash(double cash, double fee)
    {
      return  _account->cost_cash(cash, fee);
    }

    double calc_commission(const CerebroOrder &order) const
    {
        return (order.filled_price * order.filled_quantity) * _account->get_commission_rate();
    }

    double calc_frozen_cash(const CerebroOrder& order, double last_price = 0.0)
    {
        // 计算需冻结的资金
        return (order.price + _account->get_slippage()) * order.quantity;
    }

    void on_tick(const CerebroTickRecord &record); // 接收tick时间，触发订单撮合

    void on_order_update(const CerebroOrder &order);

    // 交易接口，必须返回订单号
    int64_t buy(const Symbol &symbol, double quantity, double price = 0.0);
    int64_t sell(const Symbol &symbol, double quantity, double price = 0.0);

    int64_t buy_to_cover(const Symbol &symbol, double quantity, double price = 0.0);
    int64_t sell_short(const Symbol &symbol, double quantity, double price = 0.0);

    // 撤单视作特殊的订单, 待撤订单id填写在扩展字段中
    int64_t cancel_order(int64_t order_id);

  protected:
    // 处理特殊订单
    void process_special_order(const CerebroOrder &order);
    void add_order(const CerebroOrder &order);

    CerebroPositionTracker *get_long_tracker(const Symbol &symbol)
    {
        auto it = _lpositions.find(symbol);
        if (it != _lpositions.cend())
        {
            return it->second;
        }
        auto p = new CerebroPositionTracker(symbol);
        _lpositions.insert(std::make_pair(symbol, p));
        return p;
    }
    CerebroPositionTracker *get_short_tracker(const Symbol &symbol)
    {
        auto it = _spositions.find(symbol);
        if (it != _spositions.cend())
        {
            return it->second;
        }
        auto p = new CerebroPositionTracker(symbol);
        _spositions.insert(std::make_pair(symbol, p));
        return p;
    }

    int do_order_amend(CerebroOrder &order);
    int do_order_cancel(CerebroOrder &order);
  private:
    double _cash{0};                 // 资金
    double _slippage{0};             // 滑点
    double _commission_rate{0.0003}; // 手续费
    CerebroAccountWrap* _account{nullptr};
    std::list<CerebroOrder> _unfill_orders; // 未成订单 
    std::vector<CerebroOrder> _filled_orders; // 成交订单
    std::map<Symbol, CerebroPositionTracker *> _lpositions; // 多仓
    std::map<Symbol, CerebroPositionTracker *> _spositions; // 空仓
    std::map<std::string, CerebroMatcher *> _exchg2matcher; // 交易所撮合器 
};



