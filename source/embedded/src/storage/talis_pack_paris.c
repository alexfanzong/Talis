/**
 * @file talis_pack_paris.c
 * @brief 巴黎行程的离线行动包
 *
 * ⚠️ 本文件是 UTF-8，含法语重音和中文。显示依赖 src/ui/fonts 下的
 *    lv_font_talis_*（Latin-1 + 汉字子集）。
 *    改动中文后必须重跑 tools/make_fonts.sh，否则新字显示为空白。
 *
 * 语言分工：
 *   法语正文        —— 给当地人看，必须地道，且与预录语音一致
 *   中文正文        —— 给用户核对自己正在展示什么
 *   机构名 / 地址   —— 保留原文，因为要指给当地人看
 *   其余界面文字    —— 中文
 */

#include "talis_types.h"

/* ==========================================================================
 * 手机问题 -> 手机已经丢了 -> 三个选项
 * ========================================================================== */
static const talis_node_t sg_phone_lost_children[] = {
    {
        .title = "出示求助卡",
        .hint  = "法语 · 中文 · 可朗读",
        /* 法语第一句与预录语音逐字一致 —— 用户按播放时，
         * 喇叭念的就是屏幕上这句，对方能对着文字听 */
        .body_fr = "J'ai perdu mon téléphone, aidez-moi s'il vous plaît.\n\n"
                   "Je ne peux pas accéder aux informations de mon hébergement "
                   "ni passer d'appel. Pouvez-vous m'aider à contacter le "
                   "personnel approprié ?",
        .body_zh = "我的手机丢了，请帮帮我。\n\n"
                   "我现在无法查看住宿信息，也无法打电话。"
                   "您能帮我联系相关的工作人员吗？",
        .has_audio = true,
    },
    {
        .title     = "回到住处",
        .hint      = "住宿地址",
        .place     = "Demo lodging",
        .address   = "Configure locally before travel",
        .next_step = "把这个地址给司机或路人看。到店后请前台帮忙联系家人。",
        .body_zh   = "如果身上没钱，直接回住处最稳妥。前台能借电话，"
                     "也能证明你的身份。",
    },
    {
        .title     = "找警局",
        .hint      = "Paris Centre · 报警与失物",
        .place     = "Commissariat central de Paris Centre",
        .address   = "1 bis Rue Gabriel Vicaire, 75003 Paris",
        .phone     = "17 · 紧急 112",
        .next_step = "说明手机丢失的时间和地点，问清楚由哪个部门受理，"
                     "当场记下受理编号。",
        .keep      = "受理编号、经办人姓名、下一个地址、时间",
        .body_fr   = "Mon téléphone a été perdu ou volé. "
                     "Je voudrais faire une déclaration, s'il vous plaît.",
        .body_zh   = "我的手机丢失或被盗，我想报案。",
    },
};

/* ==========================================================================
 * 手机问题
 * ========================================================================== */
static const talis_node_t sg_phone_children[] = {
    {
        .title  = "寻找我的手机",
        .hint   = "让手机响铃",
        .action = ACTION_FIND_PHONE,
    },
    {
        .title       = "手机已经丢了",
        .hint        = "求助卡 · 回住处 · 找警局",
        .children    = sg_phone_lost_children,
        .child_count = sizeof(sg_phone_lost_children) / sizeof(sg_phone_lost_children[0]),
    },
};

/* ==========================================================================
 * 医疗求助
 * ========================================================================== */
static const talis_node_t sg_medical_children[] = {
    {
        .title     = "紧急电话",
        .hint      = "15 · 112 · 17",
        .phone     = "15 急救 · 112 通用紧急 · 17 警察",
        .next_step = "说明所在位置和症状。112 支持英语。",
        .body_fr   = "J'ai besoin d'un médecin, s'il vous plaît.",
        .body_zh   = "我需要看医生，请帮忙。",
    },
    {
        .title     = "就近医院",
        .hint      = "Hôtel-Dieu AP-HP",
        .place     = "Hôtel-Dieu AP-HP",
        .address   = "1 Place du Parvis Notre-Dame, 75004 Paris",
        .next_step = "急诊入口 24 小时开放。到前台说 urgences。",
        .keep      = "就诊记录、发票、诊断书",
    },
    {
        .title     = "药房",
        .hint      = "绿色十字标识",
        .place     = "Pharmacie",
        .next_step = "常见药品可直接购买。夜间找标有 pharmacie de garde 的值班药房。",
    },
};

/* ==========================================================================
 * 领事协助
 * ========================================================================== */
static const talis_node_t sg_consular_children[] = {
    {
        .title     = "中国驻法国大使馆",
        .hint      = "护照丢失 · 旅行证",
        .place     = "Ambassade de Chine en France",
        .address   = "20 Rue Monsieur, 75007 Paris",
        .next_step = "护照丢失时前往办理旅行证。先电话确认当日办公时间。",
        .keep      = "报警受理编号、证件照、身份信息",
    },
    {
        .title     = "领事保护热线",
        .hint      = "24 小时 · +86 10 12308",
        .place     = "外交部全球领事保护与服务应急热线",
        .phone     = "+86 10 12308",
        .next_step = "24 小时接听。人身安全、证件丢失、重大伤病都可以打。",
        .body_zh   = "借用当地人的手机时，请对方直接拨 +86 10 12308。",
    },
};

/* ==========================================================================
 * 交通与失物招领
 * ========================================================================== */
static const talis_node_t sg_transport_children[] = {
    {
        .title     = "巴黎失物招领中心",
        .hint      = "全市统一汇总",
        .place     = "Service des Objets Trouvés",
        .address   = "36 Rue des Morillons, 75015 Paris",
        .next_step = "全市公共场所拾获物统一汇总到这里。需本人携带身份证明。",
        .keep      = "遗失物品描述、丢失的时间和地点",
    },
    {
        .title     = "地铁与公交",
        .hint      = "RATP",
        .place     = "RATP 失物招领",
        .next_step = "在任一地铁站服务台登记，信息会转到失物招领中心。",
        .body_zh   = "记清楚线路号、方向和大致时间，这三项决定能不能查到。",
    },
};

/* ==========================================================================
 * 银行卡与账号
 * ========================================================================== */
static const talis_node_t sg_account_children[] = {
    {
        .title     = "挂失顺序",
        .hint      = "先冻结资金，再夺回号码",
        .next_step = "① 银行卡 ② 手机号 ③ 邮箱 ④ 支付类账号",
        .body_zh   = "顺序很重要：手机号没夺回来之前，"
                     "很多账号的短信验证还在别人手里。",
    },
    {
        .title     = "保险与理赔",
        .hint      = "24 小时内报案",
        .next_step = "多数旅行险要求 24 小时内报案。先报案拿编号，回国再补材料。",
        .keep      = "报警回执、保单号、票据、设备型号与序列号",
    },
};

/* ==========================================================================
 * 模块表（首屏）
 * ========================================================================== */
static const talis_node_t sg_modules[] = {
    {
        .title       = "手机问题",
        .hint        = "找手机 · 丢失后怎么办",
        .children    = sg_phone_children,
        .child_count = sizeof(sg_phone_children) / sizeof(sg_phone_children[0]),
    },
    {
        .title       = "医疗求助",
        .hint        = "急救 · 医院 · 药房",
        .children    = sg_medical_children,
        .child_count = sizeof(sg_medical_children) / sizeof(sg_medical_children[0]),
    },
    {
        .title       = "领事协助",
        .hint        = "使馆 · 领保热线",
        .children    = sg_consular_children,
        .child_count = sizeof(sg_consular_children) / sizeof(sg_consular_children[0]),
    },
    {
        .title       = "交通与失物",
        .hint        = "失物中心 · 地铁",
        .children    = sg_transport_children,
        .child_count = sizeof(sg_transport_children) / sizeof(sg_transport_children[0]),
    },
    {
        .title       = "银行卡与账号",
        .hint        = "挂失 · 理赔",
        .children    = sg_account_children,
        .child_count = sizeof(sg_account_children) / sizeof(sg_account_children[0]),
    },
};

const talis_device_pack_t g_pack_paris = {
    .city         = "巴黎 Paris",
    .dates        = "2026年9月18–20日",
    .modules      = sg_modules,
    .module_count = sizeof(sg_modules) / sizeof(sg_modules[0]),
};
